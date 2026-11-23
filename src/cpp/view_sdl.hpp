// view_sdl.hpp — отрисовка сцены средствами SDL2 (GPU вместо по-пиксельного
// программного буфера). Headless-сборка и запись PNG по-прежнему идут через
// Renderer: там важна точная и повторяемая картинка для тестов.
#ifndef CB_VIEW_SDL_HPP
#define CB_VIEW_SDL_HPP

#ifdef CB_USE_SDL2

#include <string>
#include <unordered_map>

#include <SDL2/SDL.h>

#include "engine.hpp"
#include "renderer.hpp"

namespace cb {

class SdlView {
public:
    // renderer — уже созданный SDL-рендерер; metrics — программный рендерер,
    // из него берём размер кадра, масштаб, шрифт и ширину строки.
    SdlView(SDL_Renderer *renderer, Renderer &metrics);
    ~SdlView();

    SdlView(const SdlView &) = delete;
    SdlView &operator=(const SdlView &) = delete;

    // кадр целиком: фон, спрайты, частицы, поля ввода, тосты
    void draw_scene(Engine &engine);
    // сбросить кэши (например, при смене сцены со своей графикой)
    void clear_caches();

    int width() const { return width_; }
    int height() const { return height_; }
    double scale() const { return scale_; }

private:
    struct TextTexture {
        SDL_Texture *texture = nullptr;
        int width = 0;
        int height = 0;
        double left = 0.0;      // смещение от точки привязки (пиксели кадра)
        double top = 0.0;
    };

    void draw_sprite(Engine &engine, const Sprite &sprite);
    void draw_particles(Engine &engine);
    void draw_text(const std::string &text, double x, double y, double size,
                   unsigned rgba, const std::string &align);
    void fill_rect(double x, double y, double w, double h, unsigned rgba);
    void draw_ask_panel(Engine &engine);
    void draw_toasts(Engine &engine);

    SDL_Texture *image_texture(const ImageAsset &asset);
    const TextTexture *text_texture(const std::string &text, double size, unsigned rgba,
                                    const std::string &align);
    void free_caches();

    SDL_Renderer *renderer_ = nullptr;
    Renderer &metrics_;
    int width_ = 0;
    int height_ = 0;
    double scale_ = 1.0;
    // текстуры картинок: ключ — адрес ImageAsset (он живёт в std::map движка)
    std::unordered_map<const ImageAsset *, SDL_Texture *> image_cache_;
    std::unordered_map<std::string, TextTexture> text_cache_;
};

}  // namespace cb

#endif  // CB_USE_SDL2

#endif  // CB_VIEW_SDL_HPP
