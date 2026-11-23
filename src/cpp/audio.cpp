// audio.cpp — декодирование звуков проекта и миксер.
//
// MP3: minimp3 (один заголовок, CC0). WAV: RIFF/PCM 8/16 бит.
// Свой синтезатор музыки: короткая чиптюн-тема, чтобы музыка была без
// внешних файлов (в оригинальном проекте музыкальных файлов нет).
#include "audio.hpp"

#include <cmath>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <fstream>

#define MINIMP3_IMPLEMENTATION
#define MINIMP3_ONLY_MP3
#include "minimp3.h"

#ifdef CB_USE_SDL2
#include <SDL.h>
#endif

namespace cb {
namespace {

constexpr int kRate = 44100;
constexpr double kPi = 3.14159265358979323846;
constexpr int kChannels = 2;
constexpr size_t kMaxVoices = 12;

double note_hz(int semitones_from_a4) // 0 = A4 440 Гц
{
    return 440.0 * std::pow(2.0, semitones_from_a4 / 12.0);
}

bool read_file(const std::string &path, std::vector<unsigned char> *out)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return false;
    }
    in.seekg(0, std::ios::end);
    std::streamoff size = in.tellg();
    if (size <= 0) {
        return false;
    }
    in.seekg(0, std::ios::beg);
    out->resize(static_cast<size_t>(size));
    in.read(reinterpret_cast<char *>(out->data()), size);
    return static_cast<bool>(in);
}

bool decode_wav(const std::vector<unsigned char> &raw, SoundClip *out)
{
    if (raw.size() < 44 || std::memcmp(raw.data(), "RIFF", 4) != 0 ||
        std::memcmp(raw.data() + 8, "WAVE", 4) != 0) {
        return false;
    }
    size_t pos = 12;
    int channels = 0;
    int rate = kRate;
    int bits = 16;
    const unsigned char *data = nullptr;
    size_t data_size = 0;
    while (pos + 8 <= raw.size()) {
        const char *id = reinterpret_cast<const char *>(raw.data() + pos);
        unsigned size = raw[pos + 4] | (raw[pos + 5] << 8) | (raw[pos + 6] << 16) |
                        (static_cast<unsigned>(raw[pos + 7]) << 24);
        pos += 8;
        if (pos + size > raw.size()) {
            size = static_cast<unsigned>(raw.size() - pos);
        }
        if (std::memcmp(id, "fmt ", 4) == 0 && size >= 16) {
            channels = raw[pos + 2] | (raw[pos + 3] << 8);
            rate = raw[pos + 4] | (raw[pos + 5] << 8) | (raw[pos + 6] << 16) |
                   (raw[pos + 7] << 24);
            bits = raw[pos + 14] | (raw[pos + 15] << 8);
        } else if (std::memcmp(id, "data", 4) == 0) {
            data = raw.data() + pos;
            data_size = size;
        }
        pos += size + (size & 1);
    }
    if (!data || channels <= 0 || (bits != 8 && bits != 16)) {
        return false;
    }
    size_t bytes_per_sample = static_cast<size_t>(bits / 8);
    size_t samples = data_size / bytes_per_sample;
    size_t frames = samples / static_cast<size_t>(channels);
    out->pcm.clear();
    out->pcm.reserve(frames * kChannels);
    for (size_t f = 0; f < frames; ++f) {
        for (int c = 0; c < kChannels; ++c) {
            int src_channel = c < channels ? c : channels - 1;
            size_t index = f * static_cast<size_t>(channels) +
                           static_cast<size_t>(src_channel);
            float value = 0.0f;
            if (bits == 16) {
                short s = static_cast<short>(data[index * 2] |
                                             (data[index * 2 + 1] << 8));
                value = s / 32768.0f;
            } else {
                value = (data[index] - 128) / 128.0f;
            }
            out->pcm.push_back(value);
        }
    }
    (void)rate;
    return true;
}

bool decode_mp3(const std::vector<unsigned char> &raw, SoundClip *out)
{
    mp3dec_t decoder;
    mp3dec_init(&decoder);
    mp3d_sample_t pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
    size_t offset = 0;
    out->pcm.clear();
    int last_rate = 0;
    int last_channels = 0;
    while (offset < raw.size()) {
        mp3dec_frame_info_t info;
        int samples = mp3dec_decode_frame(&decoder, raw.data() + offset,
                                          static_cast<int>(raw.size() - offset), pcm,
                                          &info);
        if (info.frame_bytes <= 0) {
            break;                      // мусор/конец
        }
        offset += static_cast<size_t>(info.frame_bytes);
        if (samples <= 0 || info.channels <= 0) {
            continue;
        }
        last_rate = info.hz;
        last_channels = info.channels;
        for (int i = 0; i < samples; ++i) {
            for (int c = 0; c < kChannels; ++c) {
                int src = c < info.channels ? c : info.channels - 1;
                out->pcm.push_back(pcm[i * info.channels + src] / 32768.0f);
            }
        }
    }
    if (out->pcm.empty()) {
        return false;
    }
    // Ресемплинг к 44100: линейная интерполяция по кадрам.
    if (last_rate > 0 && last_rate != kRate) {
        size_t src_frames = out->pcm.size() / kChannels;
        double ratio = static_cast<double>(last_rate) / kRate;
        size_t dst_frames = static_cast<size_t>(src_frames / ratio);
        std::vector<float> resampled(dst_frames * kChannels, 0.0f);
        for (size_t f = 0; f < dst_frames; ++f) {
            double src_pos = f * ratio;
            size_t i0 = static_cast<size_t>(src_pos);
            size_t i1 = i0 + 1 < src_frames ? i0 + 1 : i0;
            double t = src_pos - static_cast<double>(i0);
            for (int c = 0; c < kChannels; ++c) {
                double a = out->pcm[i0 * kChannels + c];
                double b = out->pcm[i1 * kChannels + c];
                resampled[f * kChannels + c] = static_cast<float>(a + (b - a) * t);
            }
        }
        out->pcm.swap(resampled);
    }
    (void)last_channels;
    return true;
}

void write_le16(std::vector<unsigned char> *out, unsigned value)
{
    out->push_back(static_cast<unsigned char>(value & 0xff));
    out->push_back(static_cast<unsigned char>((value >> 8) & 0xff));
}

void write_le32(std::vector<unsigned char> *out, unsigned value)
{
    write_le16(out, value & 0xffff);
    write_le16(out, (value >> 16) & 0xffff);
}

}  // namespace

Audio::~Audio()
{
    close();
}

bool Audio::open()
{
#ifdef CB_USE_SDL2
    if (ready_) {
        return true;
    }
    if (SDL_WasInit(SDL_INIT_AUDIO) == 0 && SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::fprintf(stderr, "звук: SDL_InitSubSystem(AUDIO): %s\n", SDL_GetError());
        return false;
    }
    SDL_AudioSpec want;
    SDL_AudioSpec have;
    SDL_zero(want);
    want.freq = kRate;
    want.format = AUDIO_S16SYS;
    want.channels = kChannels;
    want.samples = 1024;
    want.callback = &Audio::mix_callback;
    want.userdata = this;
    SDL_AudioDeviceID device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (device == 0) {
        std::fprintf(stderr, "звук: SDL_OpenAudioDevice: %s\n", SDL_GetError());
        return false;
    }
    device_ = device;
    voices_.assign(kMaxVoices, Voice{});
    ready_ = true;
    SDL_PauseAudioDevice(device, 0);
    return true;
#else
    return false;                        // headless: только журнал
#endif
}

void Audio::close()
{
#ifdef CB_USE_SDL2
    if (device_ != 0) {
        SDL_CloseAudioDevice(device_);
        device_ = 0;
    }
#endif
    ready_ = false;
    voices_.clear();
}

const SoundClip *Audio::clip(const std::string &path)
{
    auto it = cache_.find(path);
    if (it != cache_.end()) {
        return it->second.get();
    }
    std::vector<unsigned char> raw;
    std::unique_ptr<SoundClip> decoded(new SoundClip());
    bool ok = false;
    if (read_file(path, &raw)) {
        bool wav = raw.size() > 12 && std::memcmp(raw.data(), "RIFF", 4) == 0;
        ok = wav ? decode_wav(raw, decoded.get()) : decode_mp3(raw, decoded.get());
    }
    if (!ok) {
        std::fprintf(stderr, "звук: не удалось прочитать %s\n", path.c_str());
        cache_[path] = nullptr;
        return nullptr;
    }
    const SoundClip *result = decoded.get();
    cache_[path] = std::move(decoded);
    return result;
}

void Audio::play(const std::string &path, double volume)
{
    played_.push_back(path);
    if (played_.size() > 512) {
        played_.erase(played_.begin());
    }
    const SoundClip *sound = clip(path);
    if (!sound || !ready_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &voice : voices_) {
        if (voice.active) {
            continue;
        }
        voice.clip = sound;
        voice.pos = 0;
        voice.loop = false;
        voice.volume = volume;
        voice.music = false;
        voice.active = true;
        return;
    }
    // Все голоса заняты — вытесняем самый старый.
    voices_[0].clip = sound;
    voices_[0].pos = 0;
    voices_[0].loop = false;
    voices_[0].volume = volume;
    voices_[0].active = true;
}

void Audio::play_looped(const std::string &path, double volume)
{
    played_.push_back(path);
    const SoundClip *sound = clip(path);
    if (!sound || !ready_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &voice : voices_) {
        if (voice.active) {
            continue;
        }
        voice.clip = sound;
        voice.pos = 0;
        voice.loop = true;
        voice.volume = volume;
        voice.music = false;
        voice.active = true;
        return;
    }
}

void Audio::stop_all()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &voice : voices_) {
        if (!voice.music) {
            voice.active = false;
        }
    }
}

void Audio::start_music(double volume)
{
    music_on_ = true;
    music_volume_ = volume;
    if (!music_) {
        music_.reset(new SoundClip());
        build_music(music_.get());
    }
    if (!ready_) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &voice : voices_) {
        if (voice.music) {
            voice.active = false;
        }
    }
    for (auto &voice : voices_) {
        if (voice.active) {
            continue;
        }
        voice.clip = music_.get();
        voice.pos = 0;
        voice.loop = true;
        voice.volume = volume;
        voice.music = true;
        voice.active = true;
        return;
    }
}

void Audio::stop_music()
{
    music_on_ = false;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &voice : voices_) {
        if (voice.music) {
            voice.active = false;
        }
    }
}

void Audio::set_music_enabled(bool on)
{
    if (on) {
        start_music(music_volume_);
    } else {
        stop_music();
    }
}

void Audio::mix_callback(void *user, unsigned char *stream, int len)
{
    Audio *self = static_cast<Audio *>(user);
    short *out = reinterpret_cast<short *>(stream);
    int samples = len / 2;               // всего сэмплов (кадры * 2 канала)
    std::lock_guard<std::mutex> lock(self->mutex_);
    // Буфер переиспользуется: alloc/free на каждый колбэк (43 раза в секунду)
    // даёт заметный джиттер, а звук идёт вместе с кадром.
    std::vector<float> &mix = self->scratch_;
    if (mix.size() != static_cast<size_t>(samples)) {
        mix.resize(static_cast<size_t>(samples), 0.0f);
    } else {
        std::fill(mix.begin(), mix.end(), 0.0f);
    }
    for (auto &voice : self->voices_) {
        if (!voice.active || !voice.clip) {
            continue;
        }
        const std::vector<float> &pcm = voice.clip->pcm;
        size_t total = pcm.size();
        if (total == 0) {
            voice.active = false;
            continue;
        }
        for (int i = 0; i < samples; ++i) {
            mix[static_cast<size_t>(i)] +=
                static_cast<float>(pcm[voice.pos] * voice.volume);
            ++voice.pos;
            if (voice.pos >= total) {
                if (voice.loop) {
                    voice.pos = 0;
                } else {
                    voice.active = false;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < samples; ++i) {
        float value = mix[static_cast<size_t>(i)];
        if (value > 0.98f) value = 0.98f;
        if (value < -0.98f) value = -0.98f;
        out[i] = static_cast<short>(value * 32767.0f);
    }
}

void Audio::build_music(SoundClip *out)
{
    // Чиптюн: Am–F–C–G, бас + арпеджио + «перкуссия».
    constexpr int kBar = 4;                        // долей в такте
    const double bpm = 104.0;
    const double beat = 60.0 / bpm;
    const int bars = 8;
    const int total_seconds = static_cast<int>(bars * kBar * beat) + 1;
    size_t total_frames = static_cast<size_t>(total_seconds * kRate);
    out->pcm.assign(total_frames * kChannels, 0.0f);

    // Аккорды: основной тон и трезвучие (полутоны от A4).
    struct Chord { int root; int third; int fifth; };
    const Chord chords[bars] = {
        {-12, -9, -5},   // Am
        {-16, -12, -9},  // F
        {-21, -17, -12}, // C
        {-14, -10, -7},  // G
        {-12, -9, -5},   // Am
        {-16, -12, -9},  // F
        {-21, -17, -12}, // C
        {-14, -10, -7},  // G
    };

    auto add = [&](size_t frame, double sample) {
        if (frame * 2 + 1 >= out->pcm.size()) {
            return;
        }
        out->pcm[frame * 2] += static_cast<float>(sample);
        out->pcm[frame * 2 + 1] += static_cast<float>(sample);
    };

    for (int bar = 0; bar < bars; ++bar) {
        const Chord &chord = chords[bar];
        int tones[3] = {chord.root, chord.third, chord.fifth};
        double bar_start = bar * kBar * beat;
        // Арпеджио шестнадцатыми по трезвучию.
        for (int step = 0; step < kBar * 4; ++step) {
            double start = bar_start + step * beat / 4.0;
            int tone = tones[step % 3] + 12;
            double freq = note_hz(tone);
            double duration = beat / 4.0 * 0.9;
            size_t first = static_cast<size_t>(start * kRate);
            size_t count = static_cast<size_t>(duration * kRate);
            for (size_t i = 0; i < count && first + i < total_frames; ++i) {
                double t = static_cast<double>(i) / kRate;
                double env = std::exp(-t * 12.0);
                double phase = std::fmod(t * freq, 1.0);
                double wave = phase < 0.25 ? 1.0 : -0.25;   // квадрат 25%
                add(first + i, wave * env * 0.10);
            }
        }
        // Бас на каждую долю.
        for (int beat_index = 0; beat_index < kBar; ++beat_index) {
            double start = bar_start + beat_index * beat;
            double freq = note_hz(chord.root - 12);
            size_t first = static_cast<size_t>(start * kRate);
            size_t count = static_cast<size_t>(beat * 0.95 * kRate);
            for (size_t i = 0; i < count && first + i < total_frames; ++i) {
                double t = static_cast<double>(i) / kRate;
                double env = std::exp(-t * 3.0);
                double phase = std::fmod(t * freq, 1.0);
                double wave = 1.0 - 4.0 * std::fabs(phase - 0.5);   // треугольник
                add(first + i, wave * env * 0.16);
            }
        }
        // «Бочка» на 1 и 3 доле.
        for (int beat_index = 0; beat_index < kBar; beat_index += 2) {
            double start = bar_start + beat_index * beat;
            size_t first = static_cast<size_t>(start * kRate);
            size_t count = static_cast<size_t>(0.12 * kRate);
            for (size_t i = 0; i < count && first + i < total_frames; ++i) {
                double t = static_cast<double>(i) / kRate;
                double freq = 120.0 * std::exp(-t * 22.0) + 45.0;
                double env = std::exp(-t * 18.0);
                add(first + i, std::sin(2.0 * kPi * freq * t) * env * 0.28);
            }
        }
    }
    // Плавное затухание в конце и в начале, чтобы петля не щёлкала.
    size_t fade = static_cast<size_t>(0.05 * kRate);
    for (size_t i = 0; i < fade && i < total_frames; ++i) {
        double k = static_cast<double>(i) / static_cast<double>(fade);
        out->pcm[i * 2] *= static_cast<float>(k);
        out->pcm[i * 2 + 1] *= static_cast<float>(k);
        size_t j = total_frames - 1 - i;
        out->pcm[j * 2] *= static_cast<float>(k);
        out->pcm[j * 2 + 1] *= static_cast<float>(k);
    }
}

bool Audio::export_music(const std::string &path)
{
    SoundClip theme;
    build_music(&theme);
    std::vector<unsigned char> wav;
    unsigned data_bytes = static_cast<unsigned>(theme.pcm.size() * 2);
    wav.insert(wav.end(), {'R', 'I', 'F', 'F'});
    write_le32(&wav, 36 + data_bytes);
    wav.insert(wav.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    write_le32(&wav, 16);
    write_le16(&wav, 1);                 // PCM
    write_le16(&wav, kChannels);
    write_le32(&wav, kRate);
    write_le32(&wav, kRate * kChannels * 2);
    write_le16(&wav, kChannels * 2);
    write_le16(&wav, 16);
    wav.insert(wav.end(), {'d', 'a', 't', 'a'});
    write_le32(&wav, data_bytes);
    for (float sample : theme.pcm) {
        float clamped = sample > 0.98f ? 0.98f : (sample < -0.98f ? -0.98f : sample);
        short value = static_cast<short>(clamped * 32767.0f);
        write_le16(&wav, static_cast<unsigned short>(value));
    }
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char *>(wav.data()),
              static_cast<std::streamsize>(wav.size()));
    return static_cast<bool>(out);
}

}  // namespace cb
