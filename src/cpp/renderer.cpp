// renderer.cpp — рисование спрайтов и текста в буфер RGBA8.
#include "renderer.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>

namespace cb {

namespace {

constexpr double kPi = 3.14159265358979323846;

// Шрифт 5x7, только латиница/цифры/пунктуация. Кириллица транслитерируется
// (см. transliterate) — в исходном проекте шрифт брался из системы Android.
struct Glyph {
    char ch;
    const char *rows;           // 7 строк по 5 символов: '#' = точка
};

const Glyph kFont[] = {
    {'A', ".###. #...# ##### #...# #...# #...# #...#"},
    {'B', "####. #...# ####. #...# #...# #...# ####."},
    {'C', ".#### #.... #.... #.... #.... #.... .####"},
    {'D', "####. #...# #...# #...# #...# #...# ####."},
    {'E', "##### #.... ####. #.... #.... #.... #####"},
    {'F', "##### #.... ####. #.... #.... #.... #...."},
    {'G', ".#### #.... #.... #.### #...# #...# .###."},
    {'H', "#...# #...# ##### #...# #...# #...# #...#"},
    {'I', "##### ..#.. ..#.. ..#.. ..#.. ..#.. #####"},
    {'J', "..### ...#. ...#. ...#. #..#. #..#. .##.."},
    {'K', "#...# #..#. ###.. #..#. #...# #...# #...#"},
    {'L', "#.... #.... #.... #.... #.... #.... #####"},
    {'M', "#...# ##.## #.#.# #...# #...# #...# #...#"},
    {'N', "#...# ##..# #.#.# #..## #...# #...# #...#"},
    {'O', ".###. #...# #...# #...# #...# #...# .###."},
    {'P', "####. #...# ####. #.... #.... #.... #...."},
    {'Q', ".###. #...# #...# #...# #.#.# #..#. .##.#"},
    {'R', "####. #...# ####. #.#.. #..#. #...# #...#"},
    {'S', ".#### #.... .###. ....# ....# ....# ####."},
    {'T', "##### ..#.. ..#.. ..#.. ..#.. ..#.. ..#.."},
    {'U', "#...# #...# #...# #...# #...# #...# .###."},
    {'V', "#...# #...# #...# #...# #...# .#.#. ..#.."},
    {'W', "#...# #...# #...# #...# #.#.# ##.## #...#"},
    {'X', "#...# .#.#. ..#.. ..#.. ..#.. .#.#. #...#"},
    {'Y', "#...# #...# .#.#. ..#.. ..#.. ..#.. ..#.."},
    {'Z', "##### ....# ...#. ..#.. .#... #.... #####"},
    {'0', ".###. #...# #..## #.#.# ##..# #...# .###."},
    {'1', "..#.. .##.. ..#.. ..#.. ..#.. ..#.. #####"},
    {'2', ".###. #...# ....# ...#. ..#.. .#... #####"},
    {'3', "##### ...#. ..#.. ...#. ....# #...# .###."},
    {'4', "...#. ..##. .#.#. #..#. ##### ...#. ...#."},
    {'5', "##### #.... ####. ....# ....# #...# .###."},
    {'6', "..##. .#... #.... ####. #...# #...# .###."},
    {'7', "##### ....# ...#. ..#.. .#... .#... .#..."},
    {'8', ".###. #...# #...# .###. #...# #...# .###."},
    {'9', ".###. #...# #...# .#### ....# ...#. .##.."},
    {'.', "..... ..... ..... ..... ..... .##.. .##.."},
    {',', "..... ..... ..... ..... .##.. .##.. ..#.."},
    {':', "..... .##.. .##.. ..... .##.. .##.. ....."},
    {'!', "..#.. ..#.. ..#.. ..#.. ..#.. ..... ..#.."},
    {'?', ".###. #...# ....# ...#. ..#.. ..... ..#.."},
    {'-', "..... ..... ..... ##### ..... ..... ....."},
    {'+', "..... ..#.. ..#.. ##### ..#.. ..#.. ....."},
    {'/', "....# ...#. ..#.. ..#.. .#... #.... ....."},
    {'%', "##..# ##..# ...#. ..#.. .#... #..## #..##"},
    {'(', "...#. ..#.. .#... .#... .#... ..#.. ...#."},
    {')', ".#... ..#.. ...#. ...#. ...#. ..#.. .#..."},
    {'#', ".#.#. ##### .#.#. .#.#. .#.#. ##### .#.#."},
    {'=', "..... ..... ##### ..... ##### ..... ....."},
    {'_', "..... ..... ..... ..... ..... ..... #####"},
    {'*', "..... #.#.# .###. ##### .###. #.#.# ....."},
    {'<', "...#. ..#.. .#... #.... .#... ..#.. ...#."},
    {'>', ".#... ..#.. ...#. ....# ...#. ..#.. .#..."},
};

const Glyph *find_glyph(char ch)
{
    for (const auto &g : kFont) {
        if (g.ch == ch) {
            return &g;
        }
    }
    return nullptr;
}

// Кириллица -> латиница, чтобы надписи оставались читаемыми без шрифта с Unicode.
std::string transliterate(const std::string &text)
{
    static const char *const map[][2] = {
        {"а", "a"}, {"б", "b"}, {"в", "v"}, {"г", "g"}, {"д", "d"}, {"е", "e"},
        {"ё", "e"}, {"ж", "zh"}, {"з", "z"}, {"и", "i"}, {"й", "y"}, {"к", "k"},
        {"л", "l"}, {"м", "m"}, {"н", "n"}, {"о", "o"}, {"п", "p"}, {"р", "r"},
        {"с", "s"}, {"т", "t"}, {"у", "u"}, {"ф", "f"}, {"х", "h"}, {"ц", "c"},
        {"ч", "ch"}, {"ш", "sh"}, {"щ", "sch"}, {"ъ", ""}, {"ы", "y"}, {"ь", ""},
        {"э", "e"}, {"ю", "yu"}, {"я", "ya"},
    };
    std::string out;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c < 0x80) {
            out.push_back(static_cast<char>(c));
            i++;
            continue;
        }
        // грубое определение двухбайтовых кириллических символов UTF-8
        std::string pair = text.substr(i, 2);
        std::string lower = pair;
        if (pair.size() == 2 && static_cast<unsigned char>(pair[0]) == 0xd0 &&
            static_cast<unsigned char>(pair[1]) >= 0x90 &&
            static_cast<unsigned char>(pair[1]) <= 0xaf) {
            // А-Я -> а-я
            lower[1] = static_cast<char>(static_cast<unsigned char>(pair[1]) + 0x20);
        } else if (pair.size() == 2 && static_cast<unsigned char>(pair[0]) == 0xd0 &&
                   static_cast<unsigned char>(pair[1]) == 0x81) {
            lower = "с";                            // ё/Ё
        } else if (pair.size() == 2 && static_cast<unsigned char>(pair[0]) == 0xd1 &&
                   static_cast<unsigned char>(pair[1]) == 0x91) {
            lower = "с";
        }
        bool replaced = false;
        for (const auto &entry : map) {
            if (lower == entry[0]) {
                std::string t = entry[1];
                if (!t.empty()) {
                    t[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(t[0])));
                }
                out += t;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            out.push_back('?');
        }
        i += 2;
    }
    return out;
}

}  // namespace

Renderer::Renderer(int width, int height)
    : width_(width), height_(height),
      pixels_(static_cast<size_t>(width) * static_cast<size_t>(height) * 4, 0)
{
}

void Renderer::clear(unsigned rgba)
{
    unsigned char r = static_cast<unsigned char>((rgba >> 24) & 0xff);
    unsigned char g = static_cast<unsigned char>((rgba >> 16) & 0xff);
    unsigned char b = static_cast<unsigned char>((rgba >> 8) & 0xff);
    unsigned char a = static_cast<unsigned char>(rgba & 0xff);
    for (size_t i = 0; i < pixels_.size(); i += 4) {
        pixels_[i + 0] = r;
        pixels_[i + 1] = g;
        pixels_[i + 2] = b;
        pixels_[i + 3] = a;
    }
    drawn_ = 0;
}

void Renderer::blend(int x, int y, unsigned rgba)
{
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }
    unsigned char sr = static_cast<unsigned char>((rgba >> 24) & 0xff);
    unsigned char sg = static_cast<unsigned char>((rgba >> 16) & 0xff);
    unsigned char sb = static_cast<unsigned char>((rgba >> 8) & 0xff);
    unsigned char sa = static_cast<unsigned char>(rgba & 0xff);
    size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(width_) +
                  static_cast<size_t>(x)) * 4;
    if (sa == 255) {
        pixels_[idx + 0] = sr;
        pixels_[idx + 1] = sg;
        pixels_[idx + 2] = sb;
        pixels_[idx + 3] = 255;
        return;
    }
    unsigned inv = 255u - sa;
    pixels_[idx + 0] = static_cast<unsigned char>((sr * sa + pixels_[idx + 0] * inv) / 255u);
    pixels_[idx + 1] = static_cast<unsigned char>((sg * sa + pixels_[idx + 1] * inv) / 255u);
    pixels_[idx + 2] = static_cast<unsigned char>((sb * sa + pixels_[idx + 2] * inv) / 255u);
    pixels_[idx + 3] = 255;
}

void Renderer::draw_image(const cb_image &img, double cx, double cy, double w, double h,
                          double rotation_deg, double alpha_pct, double brightness_pct)
{
    if (w <= 0.5 || h <= 0.5 || img.width <= 0 || img.height <= 0) {
        return;
    }
    double rad = rotation_deg * kPi / 180.0;
    double cos_a = std::cos(rad);
    double sin_a = std::sin(rad);

    // экранные координаты: сцена -> пиксели кадра
    double sx = cx + static_cast<double>(width_) / 2.0;
    double sy = static_cast<double>(height_) / 2.0 - cy;

    double half_w = w / 2.0;
    double half_h = h / 2.0;
    double extent = std::sqrt(half_w * half_w + half_h * half_h) + 2.0;
    int x0 = static_cast<int>(std::floor(sx - extent));
    int x1 = static_cast<int>(std::ceil(sx + extent));
    int y0 = static_cast<int>(std::floor(sy - extent));
    int y1 = static_cast<int>(std::ceil(sy + extent));
    x0 = std::max(0, x0);
    y0 = std::max(0, y0);
    x1 = std::min(width_ - 1, x1);
    y1 = std::min(height_ - 1, y1);

    double alpha = std::clamp(alpha_pct, 0.0, 100.0) / 100.0;
    double bright = std::clamp(brightness_pct, 0.0, 200.0) / 100.0;

    for (int py = y0; py <= y1; ++py) {
        for (int px = x0; px <= x1; ++px) {
            double dx = (px + 0.5) - sx;
            double dy = (py + 0.5) - sy;
            // обратное вращение
            double lx = dx * cos_a + dy * sin_a;
            double ly = -dx * sin_a + dy * cos_a;
            if (std::fabs(lx) > half_w || std::fabs(ly) > half_h) {
                continue;
            }
            double u = (lx + half_w) / w;          // 0..1 слева направо
            double v = (ly + half_h) / h;          // 0..1 сверху вниз
            int ix = static_cast<int>(u * img.width);
            int iy = static_cast<int>(v * img.height);
            if (ix < 0) ix = 0;
            if (iy < 0) iy = 0;
            if (ix >= img.width) ix = img.width - 1;
            if (iy >= img.height) iy = img.height - 1;
            const unsigned char *src = img.pixels +
                (static_cast<size_t>(iy) * static_cast<size_t>(img.width) +
                 static_cast<size_t>(ix)) * 4;
            double a = src[3] / 255.0 * alpha;
            if (a <= 0.002) {
                continue;
            }
            double r = std::min(255.0, src[0] * bright);
            double g = std::min(255.0, src[1] * bright);
            double b = std::min(255.0, src[2] * bright);
            unsigned rgba = (static_cast<unsigned>(r) << 24) |
                            (static_cast<unsigned>(g) << 16) |
                            (static_cast<unsigned>(b) << 8) |
                            static_cast<unsigned>(a * 255.0);
            blend(px, py, rgba);
        }
    }
}

void Renderer::draw_sprite(Engine &engine, const Sprite &sprite)
{
    if (!sprite.visible) {
        return;
    }
    const ImageRef *ref = sprite.current_image();
    if (!ref) {
        return;
    }
    const ImageAsset *asset = engine.image(ref->path);
    if (!asset || !asset->loaded) {
        return;
    }
    draw_image(asset->image, sprite.x, sprite.y,
               sprite.size / 100.0 * static_cast<double>(asset->width),
               sprite.size / 100.0 * static_cast<double>(asset->height),
               sprite.rotation, sprite.alpha, sprite.brightness);
    drawn_++;
}

void Renderer::draw_scene(Engine &engine)
{
    drawn_ = 0;
    clear(engine.background_rgba());
    for (Sprite *sprite : engine.sprites_in_draw_order()) {
        draw_sprite(engine, *sprite);
    }
    auto &fields = engine.texts();
    for (auto &pair : fields) {
        TextField &field = pair.second;
        if (!field.visible) {
            continue;
        }
        std::string text = field.value.empty() ? field.placeholder : field.value;
        if (text.empty()) {
            continue;
        }
        draw_text(text, field.x, field.y, field.size,
                  parse_hex_color(field.color, 0xffffffffu), field.align);
    }
    draw_toasts(engine);
}

void Renderer::fill_rect(double x, double y, double w, double h, unsigned rgba)
{
    double sx = x + static_cast<double>(width_) / 2.0;
    double sy = static_cast<double>(height_) / 2.0 - y;
    for (int py = static_cast<int>(sy); py < static_cast<int>(sy + h); ++py) {
        for (int px = static_cast<int>(sx); px < static_cast<int>(sx + w); ++px) {
            blend(px, py, rgba);
        }
    }
}

void Renderer::draw_text(const std::string &text, double x, double y, double size,
                         unsigned rgba, const std::string &align)
{
    std::string converted = transliterate(text);
    // размер текста в Pocket Code задаётся в «пунктах»: 24 пункта ≈ высота строки
    double scale = std::max(1.0, size / 24.0);
    double glyph_w = 6.0 * scale;
    double glyph_h = 8.0 * scale;
    double total = static_cast<double>(converted.size()) * glyph_w;
    double sx = x + static_cast<double>(width_) / 2.0;
    double sy = static_cast<double>(height_) / 2.0 - y;
    if (align == "center") {
        sx -= total / 2.0;
    } else if (align == "right") {
        sx -= total;
    }

    for (size_t i = 0; i < converted.size(); ++i) {
        const Glyph *glyph = find_glyph(static_cast<char>(std::toupper(
            static_cast<unsigned char>(converted[i]))));
        if (glyph && glyph->ch != ' ') {
            const char *rows = glyph->rows;
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[row * 6 + col] != '#') {
                        continue;
                    }
                    double px = sx + static_cast<double>(i) * glyph_w +
                                static_cast<double>(col) * scale;
                    double py = sy - static_cast<double>(row) * scale;
                    for (int yy = 0; yy < static_cast<int>(std::ceil(scale)); ++yy) {
                        for (int xx = 0; xx < static_cast<int>(std::ceil(scale)); ++xx) {
                            blend(static_cast<int>(px) + xx, static_cast<int>(py) + yy, rgba);
                        }
                    }
                }
            }
        }
        (void)glyph_h;
    }
}

void Renderer::draw_toasts(Engine &engine)
{
    int i = 0;
    for (const auto &msg : engine.toasts()) {
        double y = static_cast<double>(engine.stage_height()) / 2.0 -
                   30.0 - static_cast<double>(i) * 20.0;
        draw_text(msg, 0.0, y, 14.0, 0xffffffffu, "center");
        ++i;
    }
}

bool Renderer::save_png(const std::string &path) const
{
    cb_image img;
    img.width = width_;
    img.height = height_;
    img.pixels = const_cast<unsigned char *>(pixels_.data());
    return cb_png_save(path.c_str(), &img) == 0;
}

}  // namespace cb
