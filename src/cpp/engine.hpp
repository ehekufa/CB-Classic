// engine.hpp — состояние игрового мира: сцены, спрайты, ввод, текстовые поля.
//
// Координаты — «сценические», как в Pocket Code: (0,0) в центре сцены,
// X вправо, Y вверх, единица = 1 пиксель при масштабе 1:1.
#ifndef CB_ENGINE_HPP
#define CB_ENGINE_HPP

#include <deque>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "../c/io.h"
#include "../c/png.h"

namespace cb {

struct ImageRef {
    int id = 0;                 // идентификатор картинки в проекте Pocket Code
    std::string path;           // путь внутри assets/
    int width = 0;
    int height = 0;
};

struct ImageAsset {
    cb_image image{};
    bool loaded = false;
    bool failed = false;
    int width = 0;
    int height = 0;
};

struct SoundRef {
    int id = 0;
    std::string path;
};

struct ObjectDef {
    int id = 0;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    double rotation = 0.0;
    double size = 100.0;        // проценты
    std::vector<ImageRef> images;
    std::vector<SoundRef> sounds;
    int default_image = 0;      // индекс в images
};

struct SceneDef {
    int id = 0;
    std::string name;
    std::vector<ObjectDef> objects;
};

// Частица: отдельная точка эффекта (createRadialParticle в Pocket Code).
struct Particle {
    double x = 0.0;
    double y = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double ax = 0.0;
    double ay = 0.0;
    double age = 0.0;
    double life = 1.0;
    double size_start = 30.0;
    double size_end = 8.0;
    double rotation = 0.0;
    double rotation_speed = 0.0;
    unsigned rgba = 0xffffffffu;
};

// Источник частиц, привязанный к объекту по имени («Частицы»).
struct ParticleEmitter {
    std::string name;
    int image_id = 0;
    double x = 0.0;
    double y = 0.0;
    double rate = 12.0;          // частиц в секунду
    double emit_accum = 0.0;
    double life_min = 0.5;
    double life_max = 1.0;
    double speed_min = 40.0;
    double speed_max = 110.0;
    double size_start = 26.0;
    double size_end = 4.0;
    unsigned rgba = 0xffffffffu;
    bool active = true;
    std::vector<Particle> particles;
};

struct TextField {
    std::string label;
    double x = 0.0;
    double y = 0.0;
    double size = 12.0;
    double width = 0.0;
    double height = 0.0;
    std::string color = "#FFFFFF";
    std::string align = "left";
    std::string value;
    std::string placeholder;
    bool visible = true;
    bool editable = false;
    int max_length = 32;        // как EditText в Pocket Code
};

// Активное касание: id — идентификатор пальца от системы ввода,
// порядок в списке — порядок нажатия (1-й палец, 2-й палец, ...).
struct Touch {
    int id = 0;
    double x = 0.0;
    double y = 0.0;
};

class Engine;

// Один экземпляр объекта на сцене (или клон).
struct Sprite {
    int handle = 0;             // уникальный идентификатор экземпляра
    int object_id = 0;
    std::string name;
    double x = 0.0;
    double y = 0.0;
    double rotation = 0.0;      // градусы, 0 = вверх, по часовой стрелке
    double size = 100.0;        // проценты
    double alpha = 100.0;       // проценты
    double brightness = 100.0;  // проценты
    bool visible = true;
    int layer = 0;
    int image_index = 0;        // индекс в object->images
    const ObjectDef *def = nullptr;
    bool is_clone = false;

    const ImageRef *current_image() const;
    const ImageRef *look_override = nullptr;   // картинка-скин не из своего списка
};

class Engine {
public:
    explicit Engine(std::string asset_root);

    // --- описание проекта (вызывается из Lua при загрузке) ---
    void define_scene(int id, const std::string &name);
    void define_object(int scene_id, const ObjectDef &def);
    void finalize();

    // --- сцены ---
    bool run_scene(int id);
    int current_scene_id() const;
    std::string current_scene_name() const;
    const std::vector<SceneDef> &scenes() const { return scenes_; }
    std::string last_scene_error() const { return scene_error_; }

    // --- спрайты ---
    Sprite *find(int handle);
    Sprite *find_by_object(int object_id);
    std::vector<Sprite *> sprites_in_draw_order();
    int clone(int handle);
    bool remove(int handle);

    // --- ввод: несколько пальцев одновременно ---
    void touch_begin(int id, double x, double y);   // палец коснулся экрана
    void touch_move(int id, double x, double y);    // палец поехал
    void touch_end(int id);                         // палец отпущен
    void set_touch(bool down, double x, double y, int fingers);
    bool touch_down() const { return touch_down_; }
    double touch_x() const { return touch_x_; }
    double touch_y() const { return touch_y_; }
    int touch_count() const { return static_cast<int>(touches_.size()); }
    int last_touch_id() const { return touches_.empty() ? -1 : touches_.back().id; }
    // index — 1..N в порядке нажатия (так его понимают блоки touchXByIndex/touchXByID)
    bool touch_at(int index, double *x, double *y) const;
    // нажатия, которые ещё не разобраны Lua-рантаймом (по одному на палец)
    bool pop_press(double *x, double *y);
    void clear_touches();
    void push_touch_press(double x, double y) { touch_begin(0, x, y); }

    // --- попадания по спрайтам ---
    bool sprite_hit_pixel(Sprite &s, double x, double y);        // с учётом альфы картинки
    // setImageToId из модифицированного Pocket Code умеет брать картинку
    // любого объекта проекта — на этом построены скины
    bool set_image_by_id(Sprite &sprite, int image_id);
    bool set_image_by_name(Sprite &sprite, const std::string &name);
    std::vector<Sprite *> sprites_at(double x, double y);        // сверху вниз

    // --- текстовые поля: фокус и ввод с клавиатуры ---
    TextField *focus_text_at(double x, double y);
    void focus_text(const std::string &label);
    void blur_text();                                            // подтверждает ввод
    std::string focused_text() const { return focus_; }
    bool text_input(const std::string &utf8);
    bool text_backspace();
    bool pop_text_commit(std::string *label, std::string *value);
    double text_width(const TextField &field);
    // рендерер сообщает движку, как измерить строку настоящим шрифтом
    void set_text_measurer(std::function<double(const std::string &, double)> fn) {
        measurer_ = std::move(fn);
    }

    // --- прочее ---
    double time() const { return time_; }
    double dt() const { return dt_; }
    void advance(double dt)
    {
        time_ += dt;
        dt_ = dt;
        tick_toasts(dt);
        particles_update(dt);
    }
    int stage_width() const { return stage_w_; }
    int stage_height() const { return stage_h_; }
    void set_stage(int w, int h);
    void set_background(const std::string &hex);
    unsigned background_rgba() const { return bg_; }
    bool exit_requested() const { return exit_requested_; }
    void request_exit() { exit_requested_ = true; }

    // --- текстовые поля / тосты ---
    TextField *text(const std::string &label);
    TextField &text_ensure(const std::string &label);
    void text_remove(const std::string &label);
    std::map<std::string, TextField> &texts() { return texts_; }
    void toast(const std::string &msg);
    const std::deque<std::string> &toasts() const { return toasts_; }
    void tick_toasts(double dt);
    void pop_toast();

    // --- «Firebase» и сохранения эмулируются локально ---
    void fb_set(const std::string &key, const std::string &value);
    bool fb_get(const std::string &key, std::string *out) const;
    void store_set(const std::string &key, const std::string &value);
    bool store_get(const std::string &key, std::string *out) const;
    void store_flush();
    void store_load();

    // --- звук (в headless-режиме только журнал) ---
    void play_sound(int handle, int sound_id);
    // Щелчок кнопки: звук, приложенный к объекту, иначе «Нажатие» из проекта.
    void click_sound(int handle);
    void play_sound_name(const std::string &path);           // по имени файла
    void stop_sound_by_name(const std::string &path);
    // Звук отдаётся наружу (audio.cpp): движок не знает про SDL
    void set_sound_hook(std::function<void(const std::string &, bool loop)> hook)
    {
        sound_hook_ = std::move(hook);
    }
    void stop_sounds() { active_sounds_.clear(); }
    const std::vector<std::string> &sound_log() const { return sound_log_; }

    // --- частицы (визуальные эффекты) ---
    void particles_create(int handle, const std::string &name, int image_id);
    void particles_move(int handle, const std::string &name, double x, double y);
    void particles_stop(int handle, const std::string &name);
    void particles_update(double dt);
    std::vector<const ParticleEmitter *> particle_emitters() const;
    const ImageRef *image_by_id(int id) const;

    // --- журнал для отчёта/тестов ---
    void log(const std::string &msg);
    const std::vector<std::string> &log_lines() const { return log_lines_; }
    int unsupported_calls(const std::string &name);

    const ImageAsset *image(const std::string &path);
    std::string asset_root() const { return asset_root_; }
    void go_steps(Sprite &sprite, double steps);

private:
    std::string asset_root_;
    int stage_w_ = 720;
    int stage_h_ = 1348;
    unsigned bg_ = 0xffffffffu;   // по умолчанию белый, как в Pocket Code

    std::vector<SceneDef> scenes_;
    int current_scene_ = 0;
    std::string scene_error_;

    std::unordered_map<int, Sprite> sprites_;
    int next_handle_ = 1;
    int next_clone_handle_ = 100000;

    std::unordered_map<std::string, ImageAsset> images_;
    std::map<int, ImageRef> project_images_;     // id картинки -> картинка (все объекты)

    bool touch_down_ = false;
    double touch_x_ = 0.0;
    double touch_y_ = 0.0;
    std::vector<Touch> touches_;
    std::deque<std::pair<double, double>> presses_;

    std::function<double(const std::string &, double)> measurer_;
    std::string focus_;
    std::deque<std::pair<std::string, std::string>> text_commits_;

    double time_ = 0.0;
    double dt_ = 0.0;
    bool exit_requested_ = false;

    std::map<std::string, TextField> texts_;
    std::deque<std::string> toasts_;
    std::vector<double> toast_ttl_;          // сколько секунд ещё показывать

    std::map<std::string, std::string> fb_;
    std::map<std::string, std::string> store_;

    std::function<void(const std::string &, bool loop)> sound_hook_;
    std::map<std::pair<int, std::string>, ParticleEmitter> emitters_;
    std::vector<std::string> active_sounds_;
    std::vector<std::string> sound_log_;
    std::vector<std::string> log_lines_;
    std::map<std::string, int> unsupported_counts_;
};

// Утилиты, общие для движка и рендера.
double sprite_pixel_width(const Sprite &s);
double sprite_pixel_height(const Sprite &s);
bool sprite_hit(const Sprite &s, double px, double py);
size_t utf8_length(const std::string &text);
std::vector<unsigned> utf8_decode(const std::string &text);
std::string utf8_encode(unsigned cp);
unsigned parse_hex_color(const std::string &hex, unsigned fallback);

}  // namespace cb

#endif  // CB_ENGINE_HPP
