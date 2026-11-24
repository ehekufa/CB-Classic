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


// Кириллица: 5x7 «пикселей» на букву, как и латиница. Строчные показываются
// заглавными (шрифт без полноценной гарнитуры, но текст читается по-русски).
struct CyrGlyph {
    unsigned cp;
    const char *rows;
};

const CyrGlyph kCyr[] = {
    {0x410, "..#.. .#.#. #...# #...# ##### #...# #...#"},   // А
    {0x411, "####. #.... #.... ###.. #...# #...# ###.."},   // Б
    {0x412, "####. #...# #...# ####. #...# #...# ####."},   // В
    {0x413, "##### #.... #.... #.... #.... #.... #...."},   // Г
    {0x414, "..##. .#.#. .#.#. .#.#. .#.#. ##### #...#"},   // Д
    {0x415, "##### #.... ####. #.... #.... #.... #####"},   // Е
    {0x401, "##### #.... ####. #.... #.... #.... #####"},   // Ё
    {0x416, "#.#.# .#.#. ##### .#.#. .#.#. .#.#. #.#.#"},   // Ж
    {0x417, "###. #...# ....# ..##. ....# #...# .###."},    // З
    {0x418, "#...# #...# #..## #.#.# ##..# #...# #...#"},   // И
    {0x419, "..#.. #...# #..## #.#.# ##..# #...# #...#"},   // Й
    {0x41a, "#...# #..#. ###.. #..#. #...# #...# #...#"},   // К
    {0x41b, "..### ..#.# .#..# .#..# .#..# #...# #...#"},   // Л
    {0x41c, "#...# ##.## #.#.# #...# #...# #...# #...#"},   // М
    {0x41d, "#...# #...# ##### #...# #...# #...# #...#"},   // Н
    {0x41e, ".###. #...# #...# #...# #...# #...# .###."},   // О
    {0x41f, "##### #...# #...# #...# #...# #...# #...#"},   // П
    {0x420, "####. #...# ####. #.... #.... #.... #...."},   // Р
    {0x421, ".#### #.... #.... #.... #.... #.... .####"},   // С
    {0x422, "##### ..#.. ..#.. ..#.. ..#.. ..#.. ..#.."},   // Т
    {0x423, "#...# #...# #...# .#### ....# #...# .###."},   // У
    {0x424, "..#.. .###. #.#.# #.#.# .###. ..#.. ..#.."},   // Ф
    {0x425, "#...# #...# .#.#. ..#.. .#.#. #...# #...#"},   // Х
    {0x426, "#..#. #..#. #..#. #..#. #..#. ##### ....#"},   // Ц
    {0x427, "#...# #...# #...# .#### ....# ....# ....#"},   // Ч
    {0x428, "#.#.# #.#.# #.#.# #.#.# #.#.# #.#.# #####"},   // Ш
    {0x429, "#.#.# #.#.# #.#.# #.#.# #.#.# ##### ....#"},   // Щ
    {0x42a, "##... .#... .#... .###. .#..# .#..# .###."},   // Ъ
    {0x42b, "#...# #...# #...# ###.# #..## #..## ###.#"},   // Ы
    {0x42c, "#.... #.... #.... ###.. #...# #...# ###.."},   // Ь
    {0x42d, ".###. #...# ....# ..### ....# #...# .###."},   // Э
    {0x42e, "#..#. #.#.# #.#.# ###.# #.#.# #.#.# #..#."},   // Ю
    {0x42f, ".###. #...# #...# .#### ...#. ..#.. .#..#"},   // Я
};

const char *find_cyr(unsigned cp)
{
    if (cp >= 0x430 && cp <= 0x44f) {
        cp -= 0x20;                     // строчная -> заглавная
    } else if (cp == 0x451) {
        cp = 0x401;                     // ё -> Ё
    }
    for (const auto &g : kCyr) {
        if (g.cp == cp) {
            return g.rows;
        }
    }
    return nullptr;
}

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

Renderer::Renderer(int width, int height, double scale)
    : width_(static_cast<int>(std::lround(width * scale))),
      height_(static_cast<int>(std::lround(height * scale))), scale_(scale),
      pixels_(static_cast<size_t>(width_) * static_cast<size_t>(height_) * 4, 0)
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

bool Renderer::load_font(const std::vector<std::string> &candidates)
{
    return font_.load(candidates);
}

double Renderer::text_width(const std::string &text, double size)
{
    if (font_.loaded()) {
        return font_.measure(text, size * scale_) / scale_;
    }
    // запасной вариант: встроенный шрифт 5x7
    double scale = std::max(1.0, size / 24.0);
    return static_cast<double>(utf8_length(text)) * 6.0 * scale;
}

void Renderer::blend_alpha(int x, int y, unsigned rgba, unsigned char alpha)
{
    unsigned char base = static_cast<unsigned char>(rgba & 0xff);
    unsigned scaled = static_cast<unsigned>(alpha) * base / 255u;
    blend(x, y, (rgba & 0xffffff00u) | scaled);
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
    cx *= scale_; cy *= scale_; w *= scale_; h *= scale_;
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
            // Во сколько исходных пикселей попадает один пиксель кадра.
            // Больше 1 — картинка уменьшается: берём среднее по площадке,
            // иначе крупные фоны (снег, шум) превращаются в «рябь».
            double sample = (img.width / w > img.height / h) ? img.width / w
                                                             : img.height / h;
            int taps = 1;
            if (sample > 1.25) {
                taps = static_cast<int>(std::ceil(sample));
                if (taps > 4) taps = 4;
                if (taps < 2) taps = 2;
            }
            double acc_r = 0.0, acc_g = 0.0, acc_b = 0.0, acc_a = 0.0;
            double inv = 1.0;
            if (taps == 1) {
                // увеличение: билинейная интерполяция — без «крупных пикселей»
                double fx = u * img.width - 0.5;
                double fy = v * img.height - 0.5;
                int x0i = static_cast<int>(std::floor(fx));
                int y0i = static_cast<int>(std::floor(fy));
                double tx = fx - x0i;
                double ty = fy - y0i;
                for (int dyi = 0; dyi < 2; ++dyi) {
                    for (int dxi = 0; dxi < 2; ++dxi) {
                        int ix = x0i + dxi;
                        int iy = y0i + dyi;
                        if (ix < 0) ix = 0;
                        if (iy < 0) iy = 0;
                        if (ix >= img.width) ix = img.width - 1;
                        if (iy >= img.height) iy = img.height - 1;
                        const unsigned char *tap = img.pixels +
                            (static_cast<size_t>(iy) * static_cast<size_t>(img.width) +
                             static_cast<size_t>(ix)) * 4;
                        double weight = (dxi ? tx : 1.0 - tx) * (dyi ? ty : 1.0 - ty);
                        acc_r += tap[0] * weight;
                        acc_g += tap[1] * weight;
                        acc_b += tap[2] * weight;
                        acc_a += tap[3] * weight;
                    }
                }
            } else {
                double base_x = u * img.width - (taps - 1) * sample / 2.0;
                double base_y = v * img.height - (taps - 1) * sample / 2.0;
                for (int sy = 0; sy < taps; ++sy) {
                    for (int sx = 0; sx < taps; ++sx) {
                        int ix = static_cast<int>(base_x + sx * sample);
                        int iy = static_cast<int>(base_y + sy * sample);
                        if (ix < 0) ix = 0;
                        if (iy < 0) iy = 0;
                        if (ix >= img.width) ix = img.width - 1;
                        if (iy >= img.height) iy = img.height - 1;
                        const unsigned char *tap = img.pixels +
                            (static_cast<size_t>(iy) * static_cast<size_t>(img.width) +
                             static_cast<size_t>(ix)) * 4;
                        acc_r += tap[0];
                        acc_g += tap[1];
                        acc_b += tap[2];
                        acc_a += tap[3];
                    }
                }
                inv = 1.0 / static_cast<double>(taps * taps);
            }
            double a = acc_a * inv / 255.0 * alpha;
            if (a <= 0.002) {
                continue;
            }
            double r = std::min(255.0, acc_r * inv * bright);
            double g = std::min(255.0, acc_g * inv * bright);
            double b = std::min(255.0, acc_b * inv * bright);
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
    double vx = sprite.x;
    double vy = sprite.y;
    engine.view_xy(sprite, &vx, &vy);      // камера сцены (прокрутка списка кубов)
    draw_image(asset->image, vx, vy,
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
    draw_particles(engine);
    auto &fields = engine.texts();
    const std::string focused = engine.focused_text();
    draw_ask_panel(engine);
    // мигающая каретка: 0.5 с видна, 0.5 с нет
    bool caret_visible = std::fmod(engine.time(), 1.0) < 0.6;
    for (auto &pair : fields) {
        TextField &field = pair.second;
        if (!field.visible) {
            continue;
        }
        bool is_focused = field.editable && focused == field.label;
        std::string text = field.value.empty() ? field.placeholder : field.value;
        unsigned color = parse_hex_color(field.color, 0xffffffffu);

        if (field.editable) {
            // Полностью чёрная подложка (просьба: «все поля ввода чёрные, кроме
            // входа в аккаунт») и белый текст — иначе на чёрном ничего не видно.
            double w = field.width > 0.0 ? field.width : std::max(60.0, engine.text_width(field));
            double h = field.height > 0.0 ? field.height : std::max(28.0, field.size * 1.6);
            unsigned frame = is_focused ? 0x3a90d0ffu : 0x5a5a5affu;
            double left = field.x - w / 2.0;
            double top = field.y + h / 2.0;      // fill_rect рисует вниз от top
            fill_rect(left, top, w, h, 0x000000ffu);
            fill_rect(left, top, w, 2.0, frame);                  // верх
            fill_rect(left, field.y - h / 2.0, w, 2.0, frame);    // низ
            fill_rect(left, field.y - h / 2.0, 2.0, h, frame);    // лево
            fill_rect(left + w - 2.0, field.y - h / 2.0, 2.0, h, frame);
            color = 0xffffffffu;
        }
        if (text.empty() && !is_focused) {
            continue;
        }
        if (field.value.empty()) {
            color = (color & 0xffffff00u) | 0x88u;                // placeholder — бледнее
        }
        if (!text.empty()) {
            draw_text(text, field.x, field.y, field.size, color, field.align);
        }
        if (is_focused && caret_visible) {
            double total = text_width(field.value, field.size);
            double sx = field.x;
            if (field.align == "center") {
                sx -= total / 2.0;
            } else if (field.align == "right") {
                sx -= total;
            }
            double caret_h = field.size * 0.38;
            fill_rect(sx + total + 2.0, field.y - caret_h / 2.0, 2.0, caret_h,
                      (color & 0x00ffffffu) | 0xff000000u);
        }
    }
    draw_video(engine);
    draw_toasts(engine);
}

void Renderer::fill_rect(double x, double y, double w, double h, unsigned rgba)
{
    x *= scale_; y *= scale_; w *= scale_; h *= scale_;
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
    if (font_.loaded()) {
        // координаты — сценические (центр, Y вверх), plot рисует в буфер
        font_.draw(text, x * scale_, y * scale_, size * scale_, align,
                   [this, rgba](int px, int py, unsigned char alpha) {
                       blend_alpha(px + width_ / 2, height_ / 2 - py, rgba, alpha);
                   });
        return;
    }
    // Символы без глифа (редкая экзотика) транслитерируются в латиницу.
    x *= scale_; y *= scale_; size *= scale_;
    std::vector<const char *> cells;
    for (unsigned cp : utf8_decode(text)) {
        if (cp < 0x80) {
            int upper = std::toupper(static_cast<int>(cp));
            const Glyph *glyph = find_glyph(static_cast<char>(upper));
            cells.push_back(glyph ? glyph->rows : nullptr);
            continue;
        }
        const char *rows = find_cyr(cp);
        if (rows) {
            cells.push_back(rows);
            continue;
        }
        std::string latin = transliterate(utf8_encode(cp));
        for (char ch : latin) {
            const Glyph *glyph = find_glyph(static_cast<char>(
                std::toupper(static_cast<unsigned char>(ch))));
            cells.push_back(glyph ? glyph->rows : nullptr);
        }
    }
    // размер текста в Pocket Code задаётся в «пунктах»: 24 пункта ≈ высота строки
    double scale = std::max(1.0, size / 24.0);
    double glyph_w = 6.0 * scale;
    double glyph_h = 8.0 * scale;
    double total = static_cast<double>(cells.size()) * glyph_w;
    double sx = x + static_cast<double>(width_) / 2.0;
    // верх глифа: строка текста центрируется по вертикали вокруг y
    double sy = static_cast<double>(height_) / 2.0 - y - 3.5 * scale;
    if (align == "center") {
        sx -= total / 2.0;
    } else if (align == "right") {
        sx -= total;
    }

    for (size_t i = 0; i < cells.size(); ++i) {
        const char *rows = cells[i];
        if (rows) {
            for (int row = 0; row < 7; ++row) {
                for (int col = 0; col < 5; ++col) {
                    if (rows[row * 6 + col] != '#') {
                        continue;
                    }
                    double px = sx + static_cast<double>(i) * glyph_w +
                                static_cast<double>(col) * scale;
                    double py = sy + static_cast<double>(row) * scale;
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

void Renderer::draw_particles(Engine &engine)
{
    // Частицы (createRadialParticle): рисуются поверх спрайтов, но под текстом.
    for (const ParticleEmitter *emitter : engine.particle_emitters()) {
        const ImageRef *ref = engine.image_by_id(emitter->image_id);
        if (!ref) {
            continue;
        }
        const ImageAsset *asset = engine.image(ref->path);
        if (!asset || !asset->loaded) {
            continue;
        }
        for (const Particle &particle : emitter->particles) {
            double t = particle.age / (particle.life > 0.0 ? particle.life : 1.0);
            double size = particle.size_start +
                          (particle.size_end - particle.size_start) * t;
            // проявление / затухание
            double alpha = t < 0.15 ? t / 0.15 : 1.0 - std::max(0.0, (t - 0.65) / 0.35);
            if (alpha <= 0.02 || size <= 0.5) {
                continue;
            }
            draw_image(asset->image, particle.x, particle.y, size, size,
                       particle.rotation, alpha * 100.0, 100.0);
        }
    }
}

void Renderer::draw_ask_panel(Engine &engine)
{
    // E.ask — настоящий диалог, как в Pocket Code: вопрос крупным белым
    // текстом, поле ввода (его рисует общий цикл полей) и две кнопки
    // «Отмена»/«Принять» на тёмных плашках. Всё остальное затемнено.
    if (!engine.ask_active()) {
        return;
    }
    const double sw = static_cast<double>(engine.stage_width());
    const double sh = static_cast<double>(engine.stage_height());
    // fill_rect принимает левый-ВЕРХНИЙ угол (y идёт вниз)
    fill_rect(-sw / 2.0, sh / 2.0, sw, sh, 0x00000044u);

    const std::string &prompt = engine.ask_prompt();
    if (!prompt.empty()) {
        // тень: вопрос читается и на светлой траве, и на тёмном фоне
        draw_text(prompt, 2.0, Engine::kAskPromptY - 2.0, 110.0, 0x000000bbu, "center");
        draw_text(prompt, 0.0, Engine::kAskPromptY, 110.0, 0xffffffffu, "center");
    }

    const double bw = Engine::kAskButtonW;
    const double bh = Engine::kAskButtonH;
    const double by = Engine::kAskButtonY;
    const double dx = Engine::kAskButtonDx;
    for (int i = 0; i < 2; ++i) {
        double cx = i == 0 ? -dx : dx;
        const char *label = i == 0 ? "Отмена" : "Принять";
        // плашка рисуется «вниз» от верхней кромки
        fill_rect(cx - bw / 2.0, by + bh / 2.0, bw, bh, 0x1b1b1be6u);
        fill_rect(cx - bw / 2.0, by + bh / 2.0, bw, 2.0, 0x4a4a4affu);
        fill_rect(cx - bw / 2.0, by - bh / 2.0, bw, 2.0, 0x4a4a4affu);
        draw_text(label, cx, by, 72.0, 0xffffffffu, "center");
    }
}

void Renderer::draw_video(Engine &engine)
{
    const cb_image *frame = engine.video_image();
    if (!frame || frame->width <= 0 || frame->height <= 0) {
        return;
    }
    // как в Pocket Code: ролик растягивается на всю сцену, пропорции сохраняем
    const double sw = static_cast<double>(engine.stage_width());
    const double sh = static_cast<double>(engine.stage_height());
    const double fit = std::max(sw / frame->width, sh / frame->height);
    draw_image(*frame, 0.0, 0.0, frame->width * fit, frame->height * fit, 0.0, 100.0,
               100.0);
}

// Скруглённая плашка: три прямоугольника, вложенных друг в друга. Радиус
// подбирается по высоте, чтобы уведомление читалось как системный toast.
void Renderer::pill(double x, double top, double w, double h, unsigned rgba)
{
    // top — верхняя кромка в сценических координатах (ось Y вверх), а
    // fill_rect рисует вниз от переданной точки.
    const double r = std::min(10.0, h / 2.0);
    fill_rect(x + r, top, w - 2.0 * r, h, rgba);
    fill_rect(x + r / 2.0, top - r / 2.0, w - r, h - r, rgba);
    fill_rect(x, top - r, w, h - 2.0 * r, rgba);
}

void Renderer::draw_toasts(Engine &engine)
{
    const auto &messages = engine.toasts();
    if (messages.empty()) {
        return;
    }
    // Как системное уведомление Android (Toast): тёмно-серая скруглённая
    // плашка с белым текстом ВНИЗУ по центру, чуть выше кнопок сцены.
    // Несколько сообщений — стопкой вверх, новые ближе к краю.
    // Размер шрифта в порте — это высота поля, а буква ≈ 0.38 от неё
    // (Font::pixel_size), поэтому 64 даёт привычные для toast ~24 px.
    const double text_size = 64.0;
    const double h = 56.0;
    const double pad = 30.0;
    const double bottom = -static_cast<double>(engine.stage_height()) / 2.0 + 74.0 + h / 2.0;
    const size_t count = messages.size();
    for (size_t i = 0; i < count; ++i) {
        const std::string &msg = messages[count - 1 - i];   // новые сверху
        const double y = bottom + static_cast<double>(i) * (h + 12.0);
        const double w = text_width(msg, text_size) + 2.0 * pad;
        pill(-w / 2.0, y + h / 2.0, w, h, 0x323232f2u);
        draw_text(msg, 0.0, y, text_size, 0xffffffffu, "center");
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
