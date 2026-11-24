// audio.cpp — декодирование звуков проекта и миксер.
//
// MP3: minimp3 (один заголовок, CC0). WAV: RIFF/PCM 8/16 бит.
// Играют только звуки из самого проекта (8 MP3): фоновой музыки в оригинале
// не было, поэтому синтезированную тему убрали.
#include "audio.hpp"

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
constexpr int kChannels = 2;
constexpr size_t kMaxVoices = 12;

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
        voice.active = true;
        return;
    }
}

void Audio::stop_all()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &voice : voices_) {
        voice.active = false;
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

}  // namespace cb
