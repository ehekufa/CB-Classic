// renderer.hpp — программный рендерер кадра (RGBA8) + запись PNG.
#ifndef CB_RENDERER_HPP
#define CB_RENDERER_HPP

#include <string>
#include <vector>

#include "../c/png.h"
#include "engine.hpp"
#include "font.hpp"

namespace cb {

class Renderer {
public:
    // width/height — размер сцены (координаты Pocket Code); scale — во сколько
    // раз буфер больше сцены (на телефоне 2 — текст и спрайты рисуются в
    // разрешении экрана, а не растягиваются из 1348x720)
    Renderer(int width, int height, double scale = 1.0);

    int width() const { return width_; }
    double scale() const { return scale_; }
    int height() const { return height_; }
    const unsigned char *pixels() const { return pixels_.data(); }
    unsigned char *mutable_pixels() { return pixels_.data(); }

    void clear(unsigned rgba);
    void draw_scene(Engine &engine);
    void fill_rect(double x, double y, double w, double h, unsigned rgba);
    void draw_text(const std::string &text, double x, double y, double size,
                   unsigned rgba, const std::string &align);
    // ширина строки в пикселях сцены (для центрирования и рамок полей)
    double text_width(const std::string &text, double size);
    // настоящий шрифт (TTF); без него остаётся встроенный 5×7
    bool load_font(const std::vector<std::string> &candidates);
    bool has_font() const { return font_.loaded(); }
    // шрифт нужен SDL-виду: он растеризует строки в текстуры
    Font &font() { return font_; }
    std::string font_path() const { return font_.path(); }
    bool save_png(const std::string &path) const;

    // статистика кадра — используется в тестах
    int drawn_sprites() const { return drawn_; }

private:
    void blend(int x, int y, unsigned rgba);
    void blend_alpha(int x, int y, unsigned rgba, unsigned char alpha);
    void draw_image(const cb_image &img, double cx, double cy, double w, double h,
                    double rotation_deg, double alpha_pct, double brightness_pct);
    void draw_sprite(Engine &engine, const Sprite &sprite);
    void draw_toasts(Engine &engine);
    void draw_video(Engine &engine);
    void draw_ask_panel(Engine &engine);
    void draw_particles(Engine &engine);

    Font font_;
    int width_;
    int height_;
    double scale_ = 1.0;
    std::vector<unsigned char> pixels_;
    int drawn_ = 0;
};

}  // namespace cb

#endif  // CB_RENDERER_HPP
