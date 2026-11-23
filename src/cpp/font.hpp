// font.hpp — текст настоящим TrueType-шрифтом (кириллица, строчные, сглаживание).
//
// Растеризация — stb_truetype (third_party/stb). Если файл шрифта не найден,
// рендерер откатывается на встроенный шрифт 5×7 из renderer.cpp.
#ifndef CB_FONT_HPP
#define CB_FONT_HPP

#include <map>
#include <string>
#include <vector>

namespace cb {

struct FontGlyph {
    int width = 0;
    int height = 0;
    int xoff = 0;
    int yoff = 0;
    int advance = 0;
    std::vector<unsigned char> bitmap;   // 8-битная маска
};

class Font {
public:
    // path — файл .ttf; несколько кандидатов перебираются по очереди
    bool load(const std::vector<std::string> &candidates);
    bool loaded() const { return loaded_; }
    const std::string &path() const { return path_; }

    // высота строки в пикселях для заданного размера в «пунктах» Pocket Code
    static double pixel_size(double size) { return size * 0.38; }

    // ширина строки в пикселях сцены
    double measure(const std::string &text, double size);

    // рисует текст; plot(x, y, alpha) вызывается для каждого пикселя глифа
    template <typename Plot>
    void draw(const std::string &text, double x, double y, double size,
              const std::string &align, Plot plot)
    {
        const int px = static_cast<int>(pixel_size(size) + 0.5);
        if (!loaded_ || px <= 0) {
            return;
        }
        const double total = measure(text, size);
        double pen = x;
        if (align == "center") {
            pen -= total / 2.0;
        } else if (align == "right") {
            pen -= total;
        }
        for (unsigned cp : codepoints(text)) {
            const FontGlyph *glyph = glyph_for(cp, px);
            if (!glyph) {
                continue;
            }
            // координаты сцены: Y вверх, y — середина строки,
            // базовая линия ниже центра, строки битмапа растут вниз
            const double baseline = y - px * 0.36;
            for (int row = 0; row < glyph->height; ++row) {
                for (int col = 0; col < glyph->width; ++col) {
                    unsigned char a = glyph->bitmap[static_cast<size_t>(row) * glyph->width + col];
                    if (a == 0) {
                        continue;
                    }
                    plot(static_cast<int>(pen + glyph->xoff + col),
                         static_cast<int>(baseline - (glyph->yoff + row)), a);
                }
            }
            pen += glyph->advance;
        }
    }

private:
    const FontGlyph *glyph_for(unsigned cp, int px);
    static std::vector<unsigned> codepoints(const std::string &text);

    bool loaded_ = false;
    std::string path_;
    std::vector<unsigned char> data_;
    std::map<unsigned long long, FontGlyph> cache_;   // (px << 32) | codepoint
    void *info_ = nullptr;                            // stbtt_fontinfo
};

}  // namespace cb

#endif  // CB_FONT_HPP
