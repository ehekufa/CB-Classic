// engine.cpp — реализация состояния мира.
#include "engine.hpp"

#include "native_ui.hpp"

#include <filesystem>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cb {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

size_t utf8_length(const std::string &text)
{
    size_t count = 0;
    for (unsigned char c : text) {
        if ((c & 0xc0) != 0x80) {
            ++count;                    // байт-продолжение UTF-8 не считается
        }
    }
    return count;
}

std::vector<unsigned> utf8_decode(const std::string &text)
{
    std::vector<unsigned> out;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        unsigned cp = c;
        size_t extra = 0;
        if (c >= 0xf0) {
            cp = c & 0x07u;
            extra = 3;
        } else if (c >= 0xe0) {
            cp = c & 0x0fu;
            extra = 2;
        } else if (c >= 0xc0) {
            cp = c & 0x1fu;
            extra = 1;
        }
        ++i;
        for (size_t k = 0; k < extra && i < text.size(); ++k, ++i) {
            cp = (cp << 6) | (static_cast<unsigned char>(text[i]) & 0x3fu);
        }
        out.push_back(cp);
    }
    return out;
}

const ImageRef *Sprite::current_image() const
{
    if (look_override) {
        return look_override;
    }
    if (!def || def->images.empty()) {
        return nullptr;
    }
    int idx = image_index;
    if (idx < 0 || idx >= static_cast<int>(def->images.size())) {
        idx = 0;
    }
    return &def->images[static_cast<size_t>(idx)];
}

double sprite_pixel_width(const Sprite &s)
{
    const ImageRef *ref = s.current_image();
    return ref ? s.size / 100.0 * static_cast<double>(ref->width) : 0.0;
}

double sprite_pixel_height(const Sprite &s)
{
    const ImageRef *ref = s.current_image();
    return ref ? s.size / 100.0 * static_cast<double>(ref->height) : 0.0;
}

bool sprite_hit(const Sprite &s, double px, double py, double sx, double sy)
{
    double w = sprite_pixel_width(s);
    double h = sprite_pixel_height(s);
    if (w <= 0.0 || h <= 0.0) {
        return false;
    }
    // обратное вращение вокруг центра спрайта (sx, sy — его позиция на экране)
    double dx = px - sx;
    double dy = py - sy;
    double rad = -s.rotation * kPi / 180.0;
    double cos_a = std::cos(rad);
    double sin_a = std::sin(rad);
    double lx = dx * cos_a - dy * sin_a;
    double ly = dx * sin_a + dy * cos_a;
    return std::fabs(lx) <= w / 2.0 && std::fabs(ly) <= h / 2.0;
}

unsigned parse_hex_color(const std::string &hex, unsigned fallback)
{
    if (hex.size() < 7 || hex[0] != '#') {
        return fallback;
    }
    unsigned value = 0;
    for (int i = 1; i <= 6; ++i) {
        char c = hex[static_cast<size_t>(i)];
        int digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        } else {
            return fallback;
        }
        value = value * 16u + static_cast<unsigned>(digit);
    }
    return (value << 8) | 0xffu;
}

Engine::Engine(std::string asset_root) : asset_root_(std::move(asset_root))
{
    store_load();      // сохранения прошлых запусков (кубки, «firebase», переменные)
}

// Путь к сохранению. Переменная окружения CB_STORE уводит его в сторону —
// так тесты идут с чистого сохранения и не трогают прогресс игрока.
static std::string store_path_for(const std::string &asset_root)
{
    const char *override = std::getenv("CB_STORE");
    if (override && *override) {
        return override;
    }
    return asset_root + "/../saves/db.tsv";
}

void Engine::store_load()
{
    std::string path = store_path_for(asset_root_);
    FILE *file = std::fopen(path.c_str(), "rb");
    if (!file) {
        return;
    }
    char line[4096];
    while (std::fgets(line, sizeof(line), file)) {
        std::string text(line);
        while (!text.empty() && (text.back() == '\n' || text.back() == '\r')) {
            text.pop_back();
        }
        size_t first = text.find('\t');
        if (first == std::string::npos) {
            continue;
        }
        size_t second = text.find('\t', first + 1);
        if (second == std::string::npos) {
            continue;
        }
        std::string kind = text.substr(0, first);
        std::string key = text.substr(first + 1, second - first - 1);
        std::string value = text.substr(second + 1);
        if (kind == "fb") {
            fb_[key] = value;
        } else if (kind == "st") {
            store_[key] = value;
        }
    }
    std::fclose(file);
}

void Engine::set_stage(int w, int h)
{
    if (w > 0 && h > 0) {
        stage_w_ = w;
        stage_h_ = h;
    }
}

void Engine::define_scene(int id, const std::string &name)
{
    for (auto &scene : scenes_) {
        if (scene.id == id) {
            scene.name = name;
            return;
        }
    }
    SceneDef def;
    def.id = id;
    def.name = name;
    scenes_.push_back(def);
}

void Engine::define_object(int scene_id, const ObjectDef &def)
{
    for (const auto &image : def.images) {
        project_images_.emplace(image.id, image);   // общая таблица для скинов
    }
    for (auto &scene : scenes_) {
        if (scene.id == scene_id) {
            scene.objects.push_back(def);
            return;
        }
    }
    SceneDef scene;
    scene.id = scene_id;
    scene.name = std::to_string(scene_id);
    scene.objects.push_back(def);
    scenes_.push_back(scene);
}

void Engine::finalize()
{
    // Подгружаем размеры картинок, чтобы корректно считать размеры спрайтов.
    for (auto &scene : scenes_) {
        for (auto &object : scene.objects) {
            for (auto &image : object.images) {
                const ImageAsset *asset = this->image(image.path);
                image.width = asset ? asset->width : 0;
                image.height = asset ? asset->height : 0;
            }
        }
    }
}

const ImageAsset *Engine::image(const std::string &path)
{
    auto it = images_.find(path);
    if (it != images_.end()) {
        return &it->second;
    }
    ImageAsset asset;
    std::string full = asset_root_ + "/" + path;
    if (cb_png_load(full.c_str(), &asset.image) == 0) {
        asset.loaded = true;
        asset.width = asset.image.width;
        asset.height = asset.image.height;
    } else {
        asset.failed = true;
        log("не удалось прочитать картинку: " + path);
    }
    auto res = images_.emplace(path, std::move(asset));
    return &res.first->second;
}

void Engine::video_frame(const std::string &path)
{
    if (path.empty() || path == video_path_) {
        return;
    }
    video_stop();
    std::string full = asset_root_ + "/" + path;
    if (cb_png_load(full.c_str(), &video_image_) != 0) {
        log("не удалось прочитать кадр видео: " + path);
        return;
    }
    video_loaded_ = true;
    video_path_ = path;
}

void Engine::video_stop()
{
    if (video_loaded_) {
        cb_image_free(&video_image_);
        video_image_ = cb_image{};
        video_loaded_ = false;
    }
    video_path_.clear();
}

bool Engine::run_scene(int id)
{
    for (const auto &scene : scenes_) {
        if (scene.id != id) {
            continue;
        }
        sprites_.clear();
        texts_.clear();
        // кадр видео-интро не должен «переехать» в следующую сцену, если игрок
        // вышел из боя до конца ролика
        video_stop();
        next_handle_ = 1;
        next_clone_handle_ = 100000;
        current_scene_ = id;
        int layer = 0;
        for (const auto &object : scene.objects) {
            Sprite sprite;
            sprite.handle = next_handle_++;
            sprite.object_id = object.id;
            sprite.name = object.name;
            sprite.x = object.x;
            sprite.y = object.y;
            sprite.rotation = object.rotation;
            sprite.size = object.size * 100.0;
            sprite.layer = layer++;
            sprite.def = &object;
            sprite.image_index = object.default_image;
            sprites_[sprite.handle] = sprite;
        }
        scene_error_.clear();
        return true;
    }
    scene_error_ = "сцена " + std::to_string(id) + " не найдена";
    log(scene_error_);
    return false;
}

int Engine::current_scene_id() const
{
    return current_scene_;
}

std::string Engine::current_scene_name() const
{
    for (const auto &scene : scenes_) {
        if (scene.id == current_scene_) {
            return scene.name;
        }
    }
    return std::string();
}

Sprite *Engine::find(int handle)
{
    auto it = sprites_.find(handle);
    return it == sprites_.end() ? nullptr : &it->second;
}

Sprite *Engine::find_by_object(int object_id)
{
    for (auto &pair : sprites_) {
        if (pair.second.object_id == object_id && !pair.second.is_clone) {
            return &pair.second;
        }
    }
    return nullptr;
}

std::vector<Sprite *> Engine::sprites_in_draw_order()
{
    std::vector<Sprite *> out;
    out.reserve(sprites_.size());
    for (auto &pair : sprites_) {
        out.push_back(&pair.second);
    }
    std::sort(out.begin(), out.end(), [](const Sprite *a, const Sprite *b) {
        if (a->layer != b->layer) {
            return a->layer < b->layer;
        }
        return a->handle < b->handle;
    });
    return out;
}

int Engine::clone(int handle)
{
    Sprite *src = find(handle);
    if (!src) {
        return 0;
    }
    Sprite copy = *src;
    copy.handle = next_clone_handle_++;
    copy.is_clone = true;
    copy.layer = src->layer + 1;
    sprites_[copy.handle] = copy;
    return copy.handle;
}

bool Engine::remove(int handle)
{
    auto it = sprites_.find(handle);
    if (it == sprites_.end() || !it->second.is_clone) {
        return false;
    }
    sprites_.erase(it);
    return true;
}

void Engine::touch_begin(int id, double x, double y)
{
    for (auto &touch : touches_) {
        if (touch.id == id) {           // повторное нажатие тем же «пальцем»
            touch.x = x;
            touch.y = y;
            presses_.emplace_back(x, y);
            touch_down_ = true;
            touch_x_ = x;
            touch_y_ = y;
            return;
        }
    }
    touches_.push_back(Touch{id, x, y});
    presses_.emplace_back(x, y);
    touch_down_ = true;
    touch_x_ = x;
    touch_y_ = y;
}

void Engine::touch_move(int id, double x, double y)
{
    for (auto &touch : touches_) {
        if (touch.id == id) {
            touch.x = x;
            touch.y = y;
            touch_x_ = x;
            touch_y_ = y;
            return;
        }
    }
}

void Engine::touch_end(int id)
{
    // запоминаем, где и над чем палец отпустили: на этим занимаются скрипты
    // «onTouchObject» (в Pocket Code это именно отпускание)
    for (const Touch &touch : touches_) {
        if (touch.id != id) {
            continue;
        }
        ReleaseEvent event;
        event.x = touch.x;
        event.y = touch.y;
        for (Sprite *sprite : sprites_at(event.x, event.y)) {
            event.handles.push_back(sprite->handle);
        }
        if (releases_.size() < 64) {
            releases_.push_back(std::move(event));
        }
        break;
    }
    touches_.erase(std::remove_if(touches_.begin(), touches_.end(),
                                  [id](const Touch &t) { return t.id == id; }),
                   touches_.end());
    touch_down_ = !touches_.empty();
    if (!touches_.empty()) {
        touch_x_ = touches_.back().x;
        touch_y_ = touches_.back().y;
    }
}

void Engine::set_touch(bool down, double x, double y, int fingers)
{
    if (down) {
        touch_begin(0, x, y);
    } else {
        touch_end(0);
    }
    (void)fingers;
}

bool Engine::touch_at(int index, double *x, double *y) const
{
    if (index < 1 || index > static_cast<int>(touches_.size())) {
        return false;
    }
    const Touch &touch = touches_[static_cast<size_t>(index - 1)];
    if (x) { *x = touch.x; }
    if (y) { *y = touch.y; }
    return true;
}

bool Engine::pop_release(double *x, double *y, std::vector<int> *handles)
{
    if (releases_.empty()) {
        return false;
    }
    if (x) { *x = releases_.front().x; }
    if (y) { *y = releases_.front().y; }
    if (handles) { *handles = releases_.front().handles; }
    releases_.pop_front();
    return true;
}

bool Engine::pop_press(double *x, double *y)
{
    if (presses_.empty()) {
        return false;
    }
    if (x) { *x = presses_.front().first; }
    if (y) { *y = presses_.front().second; }
    presses_.pop_front();
    return true;
}

void Engine::clear_touches()
{
    touches_.clear();
    presses_.clear();
    touch_down_ = false;
}

bool Engine::sprite_hit_pixel(Sprite &s, double x, double y)
{
    const ImageRef *ref = s.current_image();
    if (!ref) {
        return false;
    }
    double w = sprite_pixel_width(s);
    double h = sprite_pixel_height(s);
    if (w <= 0.0 || h <= 0.0) {
        return false;
    }
    double vx = s.x;
    double vy = s.y;
    view_xy(s, &vx, &vy);
    double dx = x - vx;
    double dy = y - vy;
    double rad = -s.rotation * kPi / 180.0;
    double lx = dx * std::cos(rad) - dy * std::sin(rad);
    double ly = dx * std::sin(rad) + dy * std::cos(rad);
    if (std::fabs(lx) > w / 2.0 || std::fabs(ly) > h / 2.0) {
        return false;
    }
    const ImageAsset *asset = image(ref->path);
    if (!asset || !asset->loaded || asset->width <= 0 || asset->height <= 0) {
        return true;                    // пикселей нет — остаётся проверка рамки
    }
    int px = static_cast<int>((lx + w / 2.0) / w * asset->width);
    int py = static_cast<int>((h / 2.0 - ly) / h * asset->height);
    px = std::min(std::max(px, 0), asset->width - 1);
    py = std::min(std::max(py, 0), asset->height - 1);
    unsigned alpha = asset->image.pixels[(static_cast<size_t>(py) * asset->width + px) * 4 + 3];
    double effective = alpha * (s.alpha / 100.0);
    return effective >= 16.0;           // полностью прозрачные места не ловят нажатия
}

bool Engine::set_image_by_id(Sprite &sprite, int image_id)
{
    if (sprite.def) {
        for (size_t i = 0; i < sprite.def->images.size(); ++i) {
            if (sprite.def->images[i].id == image_id) {
                sprite.image_index = static_cast<int>(i);
                sprite.look_override = nullptr;
                return true;
            }
        }
    }
    // «setImageToId» в моде принимает и номер образа (1..N): так приходят
    // номера скинов из Firebase («1», «2», «3»)
    if (sprite.def && image_id >= 1 &&
        image_id <= static_cast<int>(sprite.def->images.size())) {
        sprite.image_index = image_id - 1;
        sprite.look_override = nullptr;
        return true;
    }
    auto it = project_images_.find(image_id);
    if (it != project_images_.end()) {
        sprite.look_override = &it->second;   // картинка другого объекта
        return true;
    }
    return false;
}

bool Engine::set_image_by_name(Sprite &sprite, const std::string &name)
{
    if (sprite.def) {
        for (size_t i = 0; i < sprite.def->images.size(); ++i) {
            if (sprite.def->images[i].path.find(name) != std::string::npos) {
                sprite.image_index = static_cast<int>(i);
                sprite.look_override = nullptr;
                return true;
            }
        }
    }
    for (const auto &pair : project_images_) {
        if (pair.second.path.find(name) != std::string::npos) {
            sprite.look_override = &pair.second;
            return true;
        }
    }
    return false;
}

void Engine::camera_focus(int handle, double x, double y)
{
    if (!find(handle)) {
        return;
    }
    camera_handle_ = handle;
    camera_fx_ = x;
    camera_fy_ = y;
}

void Engine::camera_off(int handle)
{
    Sprite *sprite = find(handle);
    if (!sprite) {
        return;
    }
    // В оригинале этот блок зовут и «выключить камеру», и «не двигать объект
    // вместе с камерой»: в проекте так помечены кнопки «Назад», стрелки
    // прокрутки и фон интерфейса — иначе они уезжали бы вместе со списком.
    sprite->camera_fixed = true;
    if (camera_handle_ == handle) {
        camera_handle_ = 0;
    }
}

void Engine::view_xy(const Sprite &s, double *x, double *y) const
{
    *x = s.x;
    *y = s.y;
    if (s.camera_fixed || camera_handle_ == 0) {
        return;
    }
    auto it = sprites_.find(camera_handle_);
    if (it == sprites_.end()) {
        return;
    }
    *x -= it->second.x + camera_fx_;
    *y -= it->second.y + camera_fy_;
}

// --- диалог E.ask ---------------------------------------------------------

void Engine::ask_open(const std::string &prompt, const std::string &label, double field_w,
                      double field_h)
{
    ask_active_ = true;
    ask_result_ = 0;
    ask_prompt_ = prompt;
    ask_label_ = label;
    TextField *field = text(label);
    if (field) {
        // поле диалога живёт по центру и не зависит от сцены
        field->x = 0.0;
        field->y = 20.0;
        field->width = field_w;
        field->height = field_h;
        field->visible = true;
        field->editable = true;
    }
    // На Android вопрос — настоящее окно (AlertDialog + EditText): системная
    // клавиатура, выделение и удаление текста. Поле движка при этом спрятано,
    // оно нужно только как место для ответа.
    ask_native_ = native_ui_available();
    if (ask_native_) {
        if (field) {
            field->visible = false;
        }
        std::string hint = field ? field->placeholder : std::string();
        std::string initial = field ? field->value : std::string();
        native_ask(ask_prompt_, hint.empty() ? "ответ" : hint, initial, "Принять", "Отмена");
    }
}

void Engine::ask_close()
{
    if (ask_native_) {
        native_ask_dismiss();
        ask_native_ = false;
    }
    if (!focus_.empty() && focus_ == ask_label_) {
        // Диалог закрылся: снимаем фокус молча — ответ уже прочитан в E.ask,
        // а «Отмена» не должна ничего записывать в переменную.
        focus_.clear();
    }
    ask_active_ = false;
    ask_result_ = 0;
    ask_prompt_.clear();
    ask_label_.clear();
}

// Ответ из системного окна Android: кладём его в то же поле, что и при вводе
// в игре, и ставим результат («Принять»/«Отмена»), как будто нажали кнопку.
void Engine::ask_native_poll()
{
    if (!ask_native_) {
        return;
    }
    std::string value;
    bool accepted = false;
    if (!native_ask_result(&value, &accepted)) {
        return;
    }
    TextField *field = text(ask_label_);
    if (field && accepted) {
        field->value = value;
    }
    ask_result_ = accepted ? 1 : -1;
}

bool Engine::ask_touch(double x, double y)
{
    if (!ask_active_) {
        return false;
    }
    if (ask_native_) {
        return true;      // окно Android само ловит касания
    }
    TextField *field = text(ask_label_);
    if (field) {
        double w = field->width > 0.0 ? field->width : 520.0;
        double h = field->height > 0.0 ? field->height : 96.0;
        if (std::fabs(x - field->x) <= w / 2.0 && std::fabs(y - field->y) <= h / 2.0) {
            focus_text(ask_label_);            // тап по полю — правим ответ
            return true;
        }
    }
    if (std::fabs(x - kAskButtonDx) <= kAskButtonW / 2.0 &&
        std::fabs(y - kAskButtonY) <= kAskButtonH / 2.0) {
        ask_result_ = 1;                       // «Принять»
        return true;
    }
    if (std::fabs(x + kAskButtonDx) <= kAskButtonW / 2.0 &&
        std::fabs(y - kAskButtonY) <= kAskButtonH / 2.0) {
        ask_result_ = -1;                      // «Отмена»
        return true;
    }
    return true;                               // диалог модальный: остальное мимо
}

std::vector<Sprite *> Engine::sprites_at(double x, double y)
{
    std::vector<Sprite *> order = sprites_in_draw_order();
    std::vector<Sprite *> hits;
    for (auto it = order.rbegin(); it != order.rend(); ++it) {   // сверху вниз
        Sprite *sprite = *it;
        if (!sprite->visible || sprite->alpha <= 1.0) {
            continue;
        }
        if (sprite_hit_pixel(*sprite, x, y)) {
            hits.push_back(sprite);
        }
    }
    if (hits.empty()) {
        // Ни у кого нет непрозрачного пикселя в точке (например, у кнопки
        // прозрачный фон и попасть надо в букву) — берём всех, кто накрывает
        // точку рамкой, сверху вниз. Спрайты во весь экран пропускаем:
        // иначе прозрачная «подложка» съедала бы все нажатия.
        double stage_area = static_cast<double>(stage_w_) * stage_h_;
        for (auto it = order.rbegin(); it != order.rend(); ++it) {
            Sprite *sprite = *it;
            if (!sprite->visible) {
                continue;
            }
            double vx = sprite->x;
            double vy = sprite->y;
            view_xy(*sprite, &vx, &vy);
            if (!sprite_hit(*sprite, x, y, vx, vy)) {
                continue;
            }
            double area = sprite_pixel_width(*sprite) * sprite_pixel_height(*sprite);
            if (stage_area > 0.0 && area > stage_area * 0.85) {
                continue;
            }
            hits.push_back(sprite);
        }
    }
    return hits;
}

double Engine::text_width(const TextField &field)
{
    const std::string &text = field.value.empty() ? field.placeholder : field.value;
    if (measurer_) {
        return measurer_(text, field.size);
    }
    // запасной вариант без шрифта: 6x8 «пикселей» на символ при size/24
    double scale = std::max(1.0, field.size / 24.0);
    return static_cast<double>(utf8_length(text)) * 6.0 * scale;
}

std::string utf8_encode(unsigned cp)
{
    std::string out;
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
    return out;
}

TextField *Engine::focus_text_at(double x, double y)
{
    TextField *best = nullptr;
    for (auto &pair : texts_) {
        TextField &field = pair.second;
        if (!field.editable || !field.visible) {
            continue;
        }
        double w = field.width > 0.0 ? field.width : std::max(60.0, text_width(field));
        double h = field.height > 0.0 ? field.height : std::max(28.0, field.size * 1.6);
        if (std::fabs(x - field.x) <= w / 2.0 + 4.0 &&
            std::fabs(y - field.y) <= h / 2.0 + 4.0) {
            best = &field;
        }
    }
    if (best) {
        focus_text(best->label);
    }
    return best;
}

void Engine::focus_text(const std::string &label)
{
    if (focus_ == label) {
        return;
    }
    if (!focus_.empty()) {
        blur_text();
    }
    TextField *field = text(label);
    if (!field || !field->editable || !field->visible) {
        return;
    }
    focus_ = label;
    log("фокус ввода: " + label);
}

void Engine::blur_text()
{
    if (focus_.empty()) {
        return;
    }
    TextField *field = text(focus_);
    std::string label = focus_;
    focus_.clear();
    if (field) {
        text_commits_.emplace_back(label, field->value);
        log("ввод подтверждён: " + label + " = [" + field->value + "]");
    }
}

bool Engine::text_input(const std::string &utf8)
{
    TextField *field = focus_.empty() ? nullptr : text(focus_);
    if (!field || !field->editable || !field->visible || utf8.empty()) {
        return false;
    }
    if (utf8_length(field->value) + utf8_length(utf8) >
        static_cast<size_t>(std::max(1, field->max_length))) {
        return false;
    }
    field->value += utf8;
    return true;
}

bool Engine::text_backspace()
{
    TextField *field = focus_.empty() ? nullptr : text(focus_);
    if (!field || field->value.empty()) {
        return false;
    }
    size_t start = field->value.size() - 1;
    while (start > 0 && (static_cast<unsigned char>(field->value[start]) & 0xc0) == 0x80) {
        --start;                        // отступаем к началу UTF-8 последовательности
    }
    field->value.erase(start);
    return true;
}

bool Engine::pop_text_commit(std::string *label, std::string *value)
{
    if (text_commits_.empty()) {
        return false;
    }
    if (label) { *label = text_commits_.front().first; }
    if (value) { *value = text_commits_.front().second; }
    text_commits_.pop_front();
    return true;
}

void Engine::set_background(const std::string &hex)
{
    bg_ = parse_hex_color(hex, bg_);
}

TextField *Engine::text(const std::string &label)
{
    auto it = texts_.find(label);
    return it == texts_.end() ? nullptr : &it->second;
}

TextField &Engine::text_ensure(const std::string &label)
{
    TextField &field = texts_[label];
    field.label = label;
    return field;
}

void Engine::text_remove(const std::string &label)
{
    if (focus_ == label) {
        blur_text();
    }
    texts_.erase(label);
}

void Engine::toast(const std::string &msg)
{
    // На телефоне уведомление показывает сам Android (android.widget.Toast),
    // на десктопе — нарисованная плашка внизу кадра.
    if (native_ui_available()) {
        log("toast (android): " + msg);
        native_toast(msg);
        return;
    }
    // Оригинал умеет выдать одно и то же сообщение несколько раз подряд
    // (по ветке на каждый промокод) — в кадре оно должно быть одно.
    for (auto &existing : toasts_) {
        if (existing == msg) {
            return;
        }
    }
    toasts_.push_back(msg);
    toast_ttl_.push_back(3.0);
    while (toasts_.size() > 4) {
        toasts_.pop_front();
        if (!toast_ttl_.empty()) {
            toast_ttl_.erase(toast_ttl_.begin());
        }
    }
    log("toast: " + msg);
}

void Engine::tick_toasts(double dt)
{
    for (size_t i = 0; i < toast_ttl_.size() && i < toasts_.size(); ++i) {
        toast_ttl_[i] -= dt;
    }
    while (!toasts_.empty() && !toast_ttl_.empty() && toast_ttl_.front() <= 0.0) {
        toasts_.pop_front();
        toast_ttl_.erase(toast_ttl_.begin());
    }
}

void Engine::pop_toast()
{
    if (!toasts_.empty()) {
        toasts_.pop_front();
        if (!toast_ttl_.empty()) {
            toast_ttl_.erase(toast_ttl_.begin());
        }
    }
}

namespace {

// Ключ игры -> путь в базе. Глобальные ключи (X1, Y1, Ивент, скины…) живут в
// узле world/, чтобы их можно было забрать одним запросом; ключи с игроком
// ("Имя/Пароль" и "Имя/Пароль/К") — как есть.
std::string fb_path_for_key(const std::string &key)
{
    std::string k = key;
    while (!k.empty() && k.back() == '/') {
        k.pop_back();
    }
    if (k.find('/') == std::string::npos) {
        return "world/" + k;
    }
    return k;
}

// Путь -> узел, который имеет смысл опрашивать целиком.
std::string fb_node_for_path(const std::string &path)
{
    if (path.rfind("world/", 0) == 0) {
        return "world";
    }
    const size_t first = path.find('/');
    if (first == std::string::npos) {
        return path;
    }
    const size_t second = path.find('/', first + 1);
    if (second == std::string::npos) {
        return path;
    }
    return path.substr(0, second);
}

}  // namespace

void Engine::fb_set(const std::string &key, const std::string &value)
{
    fb_[key] = value;
    if (net_.enabled()) {
        net_.put(fb_path_for_key(key), fb_json_of(value));
    }
    store_flush();
}

void Engine::fb_auth(const std::string &api_key, const std::string &email,
                     const std::string &password)
{
    if (!net_.enabled()) {
        log("сеть: вход в firebase не выполнен — сеть выключена");
        return;
    }
    net_.sign_in(api_key, email, password);
    log("firebase: вход " + email);
}

void Engine::fb_set_token(const std::string &token)
{
    if (!net_.enabled() || token.empty()) {
        return;
    }
    net_.set_auth_token(token);
    log("firebase: запросы пойдут с готовым токеном");
}

int Engine::fb_auth_state() const
{
    return net_.auth_state();
}

std::string Engine::fb_auth_error() const
{
    return net_.auth_error();
}

bool Engine::fb_get(const std::string &key, std::string *out) const
{
    // Первое чтение ключа подписывает нас на его узел: дальше сервер сам
    // присылает новые значения, игра просто читает локальную копию.
    if (net_.enabled()) {
        const std::string node = fb_node_for_path(fb_path_for_key(key));
        if (fb_watched_.insert(node).second) {
            net_.watch(node);
        }
    }
    auto it = fb_.find(key);
    if (it == fb_.end()) {
        return false;
    }
    *out = it->second;
    return true;
}

// Разложить пришедшее с сервера по ключам игры (вызывается на каждом кадре).
void Engine::fb_tick()
{
    FbUpdate update;
    bool changed = false;
    while (net_.pop(&update)) {
        std::string key = update.path;
        if (key.rfind("world/", 0) == 0) {
            key = key.substr(6);
        }
        if (key.empty()) {
            continue;
        }
        const std::string value = fb_text_of(update.json);
        std::string &slot = fb_[key];
        if (slot != value) {
            slot = value;
            changed = true;
        }
    }
    if (changed) {
        store_flush();
    }
}

void Engine::store_set(const std::string &key, const std::string &value)
{
    store_[key] = value;
    store_flush();
}

bool Engine::store_get(const std::string &key, std::string *out) const
{
    auto it = store_.find(key);
    if (it == store_.end()) {
        return false;
    }
    *out = it->second;
    return true;
}

void Engine::store_flush()
{
    // «Firebase» и переменные сохраняются в один простой файл: key\tvalue
    std::string data;
    for (const auto &pair : fb_) {
        data += "fb\t" + pair.first + "\t" + pair.second + "\n";
    }
    for (const auto &pair : store_) {
        data += "st\t" + pair.first + "\t" + pair.second + "\n";
    }
    std::string path = store_path_for(asset_root_);
    // каталог saves/ может отсутствовать (первый запуск на телефоне)
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
    cb_write_file(path.c_str(), data.data(), data.size());
}

void Engine::play_sound(int handle, int sound_id)
{
    Sprite *sprite = find(handle);
    std::string name = "sound_" + std::to_string(sound_id);
    if (sprite && sprite->def) {
        for (const auto &sound : sprite->def->sounds) {
            if (sound.id == sound_id) {
                name = sound.path;
                break;
            }
        }
    }
    sound_log_.push_back(name);
    active_sounds_.push_back(name);
    if (sound_log_.size() > 256) {
        sound_log_.erase(sound_log_.begin());
    }
    if (sound_hook_ && !name.empty()) {
        sound_hook_(name, false);
    }
}

void Engine::click_sound(int handle)
{
    Sprite *sprite = find(handle);
    if (sprite && sprite->def && !sprite->def->sounds.empty()) {
        // у кнопки есть свой файл — играем именно его, как в оригинале
        play_sound_name(sprite->def->sounds.front().path);
        return;
    }
    // Своего файла нет (в оригинале звук лежал у соседних объектов) —
    // щёлкаем тем же «Нажатием» из проекта.
    play_sound_name("scenes/01_menyu/011_sily/sounds/0002_nazhatie.mp3");
}

void Engine::play_sound_name(const std::string &path)
{
    play_sound(0, -1);
    // play_sound(0, -1) уже записал «sound_-1» — заменяем на настоящее имя
    if (!sound_log_.empty()) {
        sound_log_.back() = path;
    }
    if (!active_sounds_.empty()) {
        active_sounds_.back() = path;
    }
    if (sound_hook_ && !path.empty()) {
        sound_hook_(path, false);
    }
}

const ImageRef *Engine::image_by_id(int id) const
{
    auto it = project_images_.find(id);
    return it == project_images_.end() ? nullptr : &it->second;
}

void Engine::particles_create(int handle, const std::string &name, int image_id)
{
    Sprite *sprite = find(handle);
    auto &emitter = emitters_[{handle, name}];
    emitter = ParticleEmitter();
    emitter.name = name;
    emitter.image_id = image_id;
    if (sprite) {
        emitter.x = sprite->x;
        emitter.y = sprite->y;
    }
    if (image_id > 0) {
        const ImageRef *ref = image_by_id(image_id);
        if (ref) {
            log("частицы: " + name + " (" + ref->path + ")");
        }
    }
}

void Engine::particles_move(int handle, const std::string &name, double x, double y)
{
    auto it = emitters_.find({handle, name});
    if (it == emitters_.end()) {
        return;
    }
    it->second.x = x;
    it->second.y = y;
}

void Engine::particles_stop(int handle, const std::string &name)
{
    auto it = emitters_.find({handle, name});
    if (it != emitters_.end()) {
        it->second.active = false;
    }
}

void Engine::particles_update(double dt)
{
    for (auto it = emitters_.begin(); it != emitters_.end();) {
        ParticleEmitter &emitter = it->second;
        if (!emitter.active && emitter.particles.empty()) {
            it = emitters_.erase(it);
            continue;
        }
        if (emitter.active) {
            emitter.emit_accum += emitter.rate * dt;
            while (emitter.emit_accum >= 1.0) {
                emitter.emit_accum -= 1.0;
                Particle p;
                p.x = emitter.x;
                p.y = emitter.y;
                double angle = (static_cast<double>(std::rand() % 6283)) / 1000.0;
                double speed = emitter.speed_min +
                               (emitter.speed_max - emitter.speed_min) *
                                   ((std::rand() % 1000) / 1000.0);
                p.vx = std::cos(angle) * speed;
                p.vy = std::sin(angle) * speed;
                p.life = emitter.life_min + (emitter.life_max - emitter.life_min) *
                                                ((std::rand() % 1000) / 1000.0);
                p.size_start = emitter.size_start;
                p.size_end = emitter.size_end;
                p.rotation = (std::rand() % 360);
                p.rotation_speed = (std::rand() % 120) - 60.0;
                p.rgba = emitter.rgba;
                emitter.particles.push_back(p);
            }
        }
        for (auto p = emitter.particles.begin(); p != emitter.particles.end();) {
            p->age += dt;
            if (p->age >= p->life) {
                p = emitter.particles.erase(p);
                continue;
            }
            p->x += p->vx * dt;
            p->y += p->vy * dt;
            p->vx += p->ax * dt;
            p->vy += p->ay * dt;
            p->rotation += p->rotation_speed * dt;
            ++p;
        }
        ++it;
    }
}

std::vector<const ParticleEmitter *> Engine::particle_emitters() const
{
    std::vector<const ParticleEmitter *> result;
    for (const auto &entry : emitters_) {
        if (!entry.second.particles.empty()) {
            result.push_back(&entry.second);
        }
    }
    return result;
}

void Engine::stop_sound_by_name(const std::string &path)
{
    for (auto it = active_sounds_.begin(); it != active_sounds_.end();) {
        it = (*it == path) ? active_sounds_.erase(it) : it + 1;
    }
}

void Engine::go_steps(Sprite &sprite, double steps)
{
    double rad = sprite.rotation * kPi / 180.0;
    sprite.x += std::sin(rad) * steps;
    sprite.y += std::cos(rad) * steps;
}

void Engine::log(const std::string &msg)
{
    log_lines_.push_back(msg);
    if (log_lines_.size() > 4000) {
        log_lines_.erase(log_lines_.begin(), log_lines_.begin() + 1000);
    }
}

int Engine::unsupported_calls(const std::string &name)
{
    return ++unsupported_counts_[name];
}

}  // namespace cb
