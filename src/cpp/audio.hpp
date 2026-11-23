// audio.hpp — звук порта: эффекты проекта (mp3 из Pocket Code) и музыка.
//
// Звуковые файлы лежат в assets и грузятся по требованию (кеш декодированных
// сэмплов). Декодер MP3 — minimp3 (third_party/minimp3), WAV — свой.
// Воспроизведение — SDL2; без SDL (headless-сборка, тесты) модуль работает как
// «журнал звуков»: файл декодируется, но в устройство не пишется.
#ifndef CB_AUDIO_HPP
#define CB_AUDIO_HPP

#include <cstddef>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace cb {

// Декодированный звук: всегда 44100 Гц, два канала, float -1..1.
struct SoundClip {
    std::vector<float> pcm;
    size_t frames() const { return pcm.size() / 2; }
};

class Audio {
public:
    Audio() = default;
    ~Audio();

    bool open();                       // открыть устройство (нужен SDL2)
    void close();
    bool ready() const { return ready_; }

    // Эффект: проиграть файл (mp3/wav). path — полный путь к файлу.
    void play(const std::string &path, double volume = 0.9);
    void play_looped(const std::string &path, double volume = 0.9);
    void stop_all();

    // Музыка: встроенная тема (синтезируется в коде), играет по кругу.
    void start_music(double volume = 0.3);
    void stop_music();
    void set_music_enabled(bool on);
    bool music_enabled() const { return music_on_; }

    // Что уже прозвучало (для тестов и журнала, работает и без SDL).
    const std::vector<std::string> &played() const { return played_; }
    void clear_log() { played_.clear(); }

    const SoundClip *clip(const std::string &path);   // декодировать с кешем
    static void build_music(SoundClip *out);          // сгенерировать тему
    bool export_music(const std::string &path);       // сохранить тему в WAV

private:
    struct Voice {
        const SoundClip *clip = nullptr;
        size_t pos = 0;
        bool loop = false;
        double volume = 1.0;
        bool music = false;
        bool active = false;
    };

    static void mix_callback(void *user, unsigned char *stream, int len);

    bool ready_ = false;
    bool music_on_ = false;
    double music_volume_ = 0.3;
    std::vector<Voice> voices_;
    std::vector<float> scratch_;         // буфер микширования (без аллокаций в кадре)
    std::vector<std::string> played_;
    std::map<std::string, std::unique_ptr<SoundClip>> cache_;
    std::unique_ptr<SoundClip> music_;
    std::mutex mutex_;
    unsigned device_ = 0;
};

}  // namespace cb

#endif
