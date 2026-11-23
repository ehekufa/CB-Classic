// engine.cpp — реализация состояния мира.
#include "engine.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace cb {

namespace {
constexpr double kPi = 3.14159265358979323846;
}

const ImageRef *Sprite::current_image() const
{
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

bool sprite_hit(const Sprite &s, double px, double py)
{
    double w = sprite_pixel_width(s);
    double h = sprite_pixel_height(s);
    if (w <= 0.0 || h <= 0.0) {
        return false;
    }
    // обратное вращение вокруг центра спрайта
    double dx = px - s.x;
    double dy = py - s.y;
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

Engine::Engine(std::string asset_root) : asset_root_(std::move(asset_root)) {}

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

bool Engine::run_scene(int id)
{
    for (const auto &scene : scenes_) {
        if (scene.id != id) {
            continue;
        }
        sprites_.clear();
        texts_.clear();
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

void Engine::set_touch(bool down, double x, double y, int fingers)
{
    touch_down_ = down;
    touch_x_ = x;
    touch_y_ = y;
    fingers_ = fingers;
}

void Engine::push_touch_press(double x, double y)
{
    touch_down_ = true;
    touch_x_ = x;
    touch_y_ = y;
    fingers_ = std::max(1, fingers_);
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
    texts_.erase(label);
}

void Engine::toast(const std::string &msg)
{
    toasts_.push_back(msg);
    if (toasts_.size() > 6) {
        toasts_.pop_front();
    }
    log("toast: " + msg);
}

void Engine::pop_toast()
{
    if (!toasts_.empty()) {
        toasts_.pop_front();
    }
}

void Engine::fb_set(const std::string &key, const std::string &value)
{
    fb_[key] = value;
    store_flush();
}

bool Engine::fb_get(const std::string &key, std::string *out) const
{
    auto it = fb_.find(key);
    if (it == fb_.end()) {
        return false;
    }
    *out = it->second;
    return true;
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
    cb_write_file((asset_root_ + "/../saves/db.tsv").c_str(), data.data(), data.size());
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
