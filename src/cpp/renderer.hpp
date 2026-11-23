// renderer.hpp — программный рендерер кадра (RGBA8) + запись PNG.
#ifndef CB_RENDERER_HPP
#define CB_RENDERER_HPP

#include <string>
#include <vector>

#include "../c/png.h"
#include "engine.hpp"

namespace cb {

class Renderer {
public:
    Renderer(int width, int height);

    int width() const { return width_; }
    int height() const { return height_; }
    const unsigned char *pixels() const { return pixels_.data(); }
    unsigned char *mutable_pixels() { return pixels_.data(); }

    void clear(unsigned rgba);
    void draw_scene(Engine &engine);
    void fill_rect(double x, double y, double w, double h, unsigned rgba);
    void draw_text(const std::string &text, double x, double y, double size,
                   unsigned rgba, const std::string &align);
    bool save_png(const std::string &path) const;

    // статистика кадра — используется в тестах
    int drawn_sprites() const { return drawn_; }

private:
    void blend(int x, int y, unsigned rgba);
    void draw_image(const cb_image &img, double cx, double cy, double w, double h,
                    double rotation_deg, double alpha_pct, double brightness_pct);
    void draw_sprite(Engine &engine, const Sprite &sprite);
    void draw_toasts(Engine &engine);

    int width_;
    int height_;
    std::vector<unsigned char> pixels_;
    int drawn_ = 0;
};

}  // namespace cb

#endif  // CB_RENDERER_HPP
