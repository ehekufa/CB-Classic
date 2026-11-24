// view_sdl.cpp — кадр рисуется SDL2: картинки лежат в текстурах, спрайты
// выводятся через SDL_RenderCopyEx (масштаб, поворот, прозрачность — на GPU).
//
// Раньше кадр собирался в программный RGBA-буфер (renderer.cpp) и целиком
// уходил в SDL_UpdateTexture. На телефоне это ~2 миллиона пикселей с
// билинейной выборкой на CPU: 70–260 мс на кадр, игра «замирала». Здесь та же
// сцена выводится текстурами, и кадр укладывается в 16 мс.
#include "view_sdl.hpp"

#ifdef CB_USE_SDL2

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

#include "../c/png.h"

namespace cb {

namespace {

Uint8 channel(unsigned rgba, int shift)
{
    return static_cast<Uint8>((rgba >> shift) & 0xffu);
}

}  // namespace

SdlView::SdlView(SDL_Renderer *renderer, Renderer &metrics)
    : renderer_(renderer), metrics_(metrics), width_(metrics.width()),
      height_(metrics.height()), scale_(metrics.scale())
{
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
}

SdlView::~SdlView() { free_caches(); }

void SdlView::free_caches()
{
    for (auto &entry : image_cache_) {
        if (entry.second) {
            SDL_DestroyTexture(entry.second);
        }
    }
    image_cache_.clear();
    if (video_texture_) {
        SDL_DestroyTexture(video_texture_);
        video_texture_ = nullptr;
    }
    for (auto &entry : text_cache_) {
        if (entry.second.texture) {
            SDL_DestroyTexture(entry.second.texture);
        }
    }
    text_cache_.clear();
}

void SdlView::clear_caches() { free_caches(); }

SDL_Texture *SdlView::image_texture(const ImageAsset &asset)
{
    auto it = image_cache_.find(&asset);
    if (it != image_cache_.end()) {
        return it->second;
    }
    const cb_image &img = asset.image;
    if (!asset.loaded || img.width <= 0 || img.height <= 0 || !img.pixels) {
        image_cache_[&asset] = nullptr;
        return nullptr;
    }
    SDL_Texture *texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STATIC, img.width, img.height);
    if (!texture) {
        image_cache_[&asset] = nullptr;
        return nullptr;
    }
    SDL_UpdateTexture(texture, nullptr, img.pixels, img.width * 4);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    image_cache_[&asset] = texture;
    return texture;
}

void SdlView::fill_rect(double x, double y, double w, double h, unsigned rgba)
{
    if (w <= 0.0 || h <= 0.0) {
        return;
    }
    SDL_Rect rect;
    // координаты сцены: x — от центра, y — вверх; fill_rect рисует вниз от top
    rect.x = static_cast<int>(std::lround(x * scale_ + width_ / 2.0));
    rect.y = static_cast<int>(std::lround(height_ / 2.0 - y * scale_));
    rect.w = std::max(1, static_cast<int>(std::lround(w * scale_)));
    rect.h = std::max(1, static_cast<int>(std::lround(h * scale_)));
    SDL_SetRenderDrawColor(renderer_, channel(rgba, 24), channel(rgba, 16),
                           channel(rgba, 8), channel(rgba, 0));
    SDL_RenderFillRect(renderer_, &rect);
}

const SdlView::TextTexture *SdlView::text_texture(const std::string &text, double size,
                                                 unsigned rgba, const std::string &align)
{
    if (text.empty()) {
        return nullptr;
    }
    const int px = static_cast<int>(Font::pixel_size(size * scale_) + 0.5);
    if (px <= 0) {
        return nullptr;
    }
    std::string key = text;
    key += '\x1f';
    key += std::to_string(px);
    key += '\x1f';
    key += std::to_string(rgba);
    key += '\x1f';
    key += align;
    auto it = text_cache_.find(key);
    if (it != text_cache_.end()) {
        return it->second.texture ? &it->second : nullptr;
    }
    // кэш строк: диалоги ввода перерисовываются на каждую букву
    if (text_cache_.size() > 384) {
        free_caches();
    }
    Font &font = metrics_.font();
    if (!font.loaded()) {
        TextTexture empty;
        text_cache_.emplace(key, empty);
        return nullptr;
    }
    double total = metrics_.text_width(text, size) * scale_;
    double dx = 0.0;
    if (align == "center") {
        dx = -total / 2.0;
    } else if (align == "right") {
        dx = -total;
    }
    const double font_size = size * scale_;
    int min_x = 0;
    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
    bool any = false;
    font.draw(text, dx, 0.0, font_size, "left",
              [&](int px_x, int px_y, unsigned char alpha) {
                  if (alpha == 0) {
                      return;
                  }
                  if (!any) {
                      min_x = max_x = px_x;
                      min_y = max_y = px_y;
                      any = true;
                      return;
                  }
                  min_x = std::min(min_x, px_x);
                  max_x = std::max(max_x, px_x);
                  min_y = std::min(min_y, px_y);
                  max_y = std::max(max_y, px_y);
              });
    if (!any) {
        TextTexture empty;
        text_cache_.emplace(key, empty);
        return nullptr;
    }
    const int w = max_x - min_x + 1;
    const int h = max_y - min_y + 1;
    std::vector<unsigned char> buffer(static_cast<size_t>(w) * h * 4, 0);
    const unsigned char color_a = static_cast<unsigned char>(rgba & 0xffu);
    font.draw(text, dx, 0.0, font_size, "left",
              [&](int px_x, int px_y, unsigned char alpha) {
                  if (alpha == 0) {
                      return;
                  }
                  const int x = px_x - min_x;
                  const int y = max_y - px_y;          // y в сцене растёт вверх
                  if (x < 0 || y < 0 || x >= w || y >= h) {
                      return;
                  }
                  unsigned char *pixel = buffer.data() + (static_cast<size_t>(y) * w + x) * 4;
                  pixel[0] = static_cast<unsigned char>((rgba >> 24) & 0xffu);
                  pixel[1] = static_cast<unsigned char>((rgba >> 16) & 0xffu);
                  pixel[2] = static_cast<unsigned char>((rgba >> 8) & 0xffu);
                  pixel[3] = static_cast<unsigned char>(alpha * color_a / 255u);
              });
    SDL_Texture *texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                                             SDL_TEXTUREACCESS_STATIC, w, h);
    if (!texture) {
        TextTexture empty;
        text_cache_.emplace(key, empty);
        return nullptr;
    }
    SDL_UpdateTexture(texture, nullptr, buffer.data(), w * 4);
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    TextTexture entry;
    entry.texture = texture;
    entry.width = w;
    entry.height = h;
    entry.left = min_x;
    entry.top = max_y;
    auto res = text_cache_.emplace(key, entry);
    return &res.first->second;
}

void SdlView::draw_text(const std::string &text, double x, double y, double size,
                        unsigned rgba, const std::string &align)
{
    const TextTexture *entry = text_texture(text, size, rgba, align);
    if (!entry) {
        return;
    }
    SDL_Rect dst;
    dst.x = static_cast<int>(std::lround(x * scale_ + width_ / 2.0 + entry->left));
    dst.y = static_cast<int>(std::lround(height_ / 2.0 - (y * scale_ + entry->top)));
    dst.w = entry->width;
    dst.h = entry->height;
    SDL_RenderCopy(renderer_, entry->texture, nullptr, &dst);
}

void SdlView::draw_sprite(Engine &engine, const Sprite &sprite)
{
    if (!sprite.visible || sprite.alpha <= 0.5) {
        return;
    }
    const ImageRef *ref = sprite.current_image();
    if (!ref) {
        return;
    }
    const ImageAsset *asset = engine.image(ref->path);
    if (!asset) {
        return;
    }
    SDL_Texture *texture = image_texture(*asset);
    if (!texture) {
        return;
    }
    const double w = sprite.size / 100.0 * static_cast<double>(asset->width) * scale_;
    const double h = sprite.size / 100.0 * static_cast<double>(asset->height) * scale_;
    if (w < 1.0 || h < 1.0) {
        return;
    }
    const double cx = sprite.x * scale_ + width_ / 2.0;
    const double cy = height_ / 2.0 - sprite.y * scale_;
    SDL_Rect dst;
    dst.x = static_cast<int>(std::lround(cx - w / 2.0));
    dst.y = static_cast<int>(std::lround(cy - h / 2.0));
    dst.w = std::max(1, static_cast<int>(std::lround(w)));
    dst.h = std::max(1, static_cast<int>(std::lround(h)));

    int alpha = static_cast<int>(std::lround(std::clamp(sprite.alpha, 0.0, 100.0) * 2.55));
    SDL_SetTextureAlphaMod(texture, static_cast<Uint8>(alpha));
    int bright = static_cast<int>(std::lround(
        std::clamp(sprite.brightness, 0.0, 200.0) * 2.55));
    bright = std::clamp(bright, 0, 255);
    SDL_SetTextureColorMod(texture, static_cast<Uint8>(bright),
                           static_cast<Uint8>(bright), static_cast<Uint8>(bright));
    SDL_RenderCopyEx(renderer_, texture, nullptr, &dst, sprite.rotation, nullptr,
                     SDL_FLIP_NONE);
}

void SdlView::draw_particles(Engine &engine)
{
    for (const ParticleEmitter *emitter : engine.particle_emitters()) {
        const ImageRef *ref = engine.image_by_id(emitter->image_id);
        if (!ref) {
            continue;
        }
        const ImageAsset *asset = engine.image(ref->path);
        if (!asset) {
            continue;
        }
        SDL_Texture *texture = image_texture(*asset);
        if (!texture) {
            continue;
        }
        for (const Particle &particle : emitter->particles) {
            const double t = particle.age / (particle.life > 0.0 ? particle.life : 1.0);
            const double size = particle.size_start +
                                (particle.size_end - particle.size_start) * t;
            double alpha = t < 0.15 ? t / 0.15 : 1.0 - std::max(0.0, (t - 0.65) / 0.35);
            if (alpha <= 0.02 || size <= 0.5) {
                continue;
            }
            const double w = size * scale_;
            const double cx = particle.x * scale_ + width_ / 2.0;
            const double cy = height_ / 2.0 - particle.y * scale_;
            SDL_Rect dst;
            dst.x = static_cast<int>(std::lround(cx - w / 2.0));
            dst.y = static_cast<int>(std::lround(cy - w / 2.0));
            dst.w = std::max(1, static_cast<int>(std::lround(w)));
            dst.h = dst.w;
            SDL_SetTextureAlphaMod(texture,
                                   static_cast<Uint8>(std::lround(alpha * 255.0)));
            SDL_SetTextureColorMod(texture, 255, 255, 255);
            SDL_RenderCopyEx(renderer_, texture, nullptr, &dst, particle.rotation, nullptr,
                             SDL_FLIP_NONE);
        }
    }
}

void SdlView::draw_ask_panel(Engine &engine)
{
    for (auto &entry : engine.texts()) {
        TextField &field = entry.second;
        if (!field.visible || entry.first.rfind("Вопрос:", 0) != 0) {
            continue;
        }
        const size_t first = entry.first.find(':') + 1;
        const size_t last = entry.first.rfind(':');
        std::string prompt =
            last > first ? entry.first.substr(first, last - first) : std::string();
        const double sw = static_cast<double>(engine.stage_width());
        const double sh = static_cast<double>(engine.stage_height());
        fill_rect(-sw / 2.0, sh / 2.0, sw, sh, 0x0000008cu);
        fill_rect(-380.0, 125.0, 760.0, 250.0, 0x1b3c5affu);
        fill_rect(-380.0, 125.0, 760.0, 3.0, 0x3a90d0ffu);
        fill_rect(-380.0, 10.0, 760.0, 2.0, 0x3a90d0ffu);
        if (!prompt.empty()) {
            draw_text(prompt, 0.0, 62.0, 62.0, 0xffffffffu, "center");
        }
        return;
    }
}

void SdlView::draw_video(Engine &engine)
{
    const cb_image *frame = engine.video_image();
    if (!frame || frame->width <= 0 || frame->height <= 0) {
        return;
    }
    // Текстуру держим одну на весь ролик: кадры интро идут 12 раз в секунду, и
    // создавать/уничтожать текстуру на каждый кадр — лишние рывки на телефоне.
    if (!video_texture_ || video_width_ != frame->width || video_height_ != frame->height) {
        if (video_texture_) {
            SDL_DestroyTexture(video_texture_);
            video_texture_ = nullptr;
        }
        video_texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA32,
                                           SDL_TEXTUREACCESS_STREAMING, frame->width,
                                           frame->height);
        if (!video_texture_) {
            return;
        }
        SDL_SetTextureBlendMode(video_texture_, SDL_BLENDMODE_BLEND);
        video_width_ = frame->width;
        video_height_ = frame->height;
    }
    SDL_UpdateTexture(video_texture_, nullptr, frame->pixels, frame->width * 4);
    const double sw = static_cast<double>(engine.stage_width());
    const double sh = static_cast<double>(engine.stage_height());
    const double fit = std::max(sw / frame->width, sh / frame->height);
    const double w = frame->width * fit * scale_;
    const double h = frame->height * fit * scale_;
    SDL_Rect dst;
    dst.x = static_cast<int>(std::lround(width_ / 2.0 - w / 2.0));
    dst.y = static_cast<int>(std::lround(height_ / 2.0 - h / 2.0));
    dst.w = static_cast<int>(std::lround(w));
    dst.h = static_cast<int>(std::lround(h));
    SDL_RenderCopy(renderer_, video_texture_, nullptr, &dst);
}

void SdlView::draw_toasts(Engine &engine)
{
    const auto &messages = engine.toasts();
    if (messages.empty()) {
        return;
    }
    const double bottom = -static_cast<double>(engine.stage_height()) / 2.0 + 34.0;
    const size_t count = messages.size();
    for (size_t i = 0; i < count; ++i) {
        const std::string &msg = messages[count - 1 - i];
        const double y = bottom + static_cast<double>(i) * 34.0;
        const double w = metrics_.text_width(msg, 20.0) + 28.0;
        fill_rect(-w / 2.0, y + 16.0, w, 32.0, 0x1b3c5ab0u);
        draw_text(msg, 0.0, y, 20.0, 0xffffffffu, "center");
    }
}

void SdlView::draw_scene(Engine &engine)
{
    const unsigned background = engine.background_rgba();
    SDL_SetRenderDrawColor(renderer_, channel(background, 24), channel(background, 16),
                           channel(background, 8), channel(background, 0));
    SDL_RenderClear(renderer_);

    for (Sprite *sprite : engine.sprites_in_draw_order()) {
        draw_sprite(engine, *sprite);
    }
    draw_particles(engine);

    const std::string focused = engine.focused_text();
    const bool caret_visible = std::fmod(engine.time(), 1.0) < 0.6;
    draw_ask_panel(engine);
    for (auto &pair : engine.texts()) {
        TextField &field = pair.second;
        if (!field.visible) {
            continue;
        }
        const bool is_focused = field.editable && focused == field.label;
        std::string text = field.value.empty() ? field.placeholder : field.value;
        unsigned color = parse_hex_color(field.color, 0xffffffffu);
        if (field.editable) {
            const double w = field.width > 0.0 ? field.width
                                               : std::max(60.0, engine.text_width(field));
            const double h = field.height > 0.0 ? field.height
                                                : std::max(28.0, field.size * 1.6);
            const unsigned frame = is_focused ? 0x3a90d0ffu : 0x9aa0a880u;
            const double left = field.x - w / 2.0;
            const double top = field.y + h / 2.0;
            fill_rect(left, top, w, h, is_focused ? 0xffffffe6u : 0xffffffb4u);
            fill_rect(left, top, w, 2.0, frame);
            fill_rect(left, field.y - h / 2.0, w, 2.0, frame);
            fill_rect(left, field.y - h / 2.0, 2.0, h, frame);
            fill_rect(left + w - 2.0, field.y - h / 2.0, 2.0, h, frame);
        }
        if (text.empty() && !is_focused) {
            continue;
        }
        if (field.value.empty()) {
            color = (color & 0xffffff00u) | 0x88u;
        }
        if (!text.empty()) {
            draw_text(text, field.x, field.y, field.size, color, field.align);
        }
        if (is_focused && caret_visible) {
            const double total = metrics_.text_width(field.value, field.size);
            double sx = field.x;
            if (field.align == "center") {
                sx -= total / 2.0;
            } else if (field.align == "right") {
                sx -= total;
            }
            const double caret_h = field.size * 0.38;
            fill_rect(sx + total + 2.0, field.y + caret_h / 2.0, 2.0, caret_h,
                      (color & 0x00ffffffu) | 0xff000000u);
        }
    }
    draw_video(engine);
    draw_toasts(engine);
}

}  // namespace cb

#endif  // CB_USE_SDL2
