// font.cpp — загрузка TTF и растеризация глифов через stb_truetype.
#include "font.hpp"

#include <cstdio>
#include <fstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace cb {

namespace {

struct FontInfo {
    stbtt_fontinfo info;
    float scale = 1.0f;
};

}  // namespace

std::vector<unsigned> Font::codepoints(const std::string &text)
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

bool Font::load(const std::vector<std::string> &candidates)
{
    if (loaded_) {
        return true;
    }
    for (const std::string &candidate : candidates) {
        std::ifstream in(candidate, std::ios::binary);
        if (!in) {
            continue;
        }
        data_.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        if (data_.size() < 1024) {
            data_.clear();
            continue;
        }
        auto *storage = new FontInfo();
        int offset = stbtt_GetFontOffsetForIndex(data_.data(), 0);
        if (offset < 0 || !stbtt_InitFont(&storage->info, data_.data(), offset)) {
            delete storage;
            data_.clear();
            continue;
        }
        info_ = storage;
        path_ = candidate;
        loaded_ = true;
        return true;
    }
    return false;
}

const FontGlyph *Font::glyph_for(unsigned cp, int px)
{
    unsigned long long key = (static_cast<unsigned long long>(px) << 32) | cp;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        return &it->second;
    }
    if (!info_) {
        return nullptr;
    }
    auto *storage = static_cast<FontInfo *>(info_);
    const float scale = stbtt_ScaleForPixelHeight(&storage->info, static_cast<float>(px));
    int advance = 0;
    int left_side = 0;
    stbtt_GetCodepointHMetrics(&storage->info, static_cast<int>(cp), &advance, &left_side);

    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;
    stbtt_GetCodepointBitmapBox(&storage->info, static_cast<int>(cp), scale, scale, &x0, &y0, &x1, &y1);

    FontGlyph glyph;
    glyph.width = x1 - x0;
    glyph.height = y1 - y0;
    glyph.xoff = x0;
    glyph.yoff = y0;
    glyph.advance = static_cast<int>(advance * scale + 0.5f);
    if (glyph.width > 0 && glyph.height > 0) {
        glyph.bitmap.resize(static_cast<size_t>(glyph.width) * glyph.height);
        stbtt_MakeCodepointBitmap(&storage->info, glyph.bitmap.data(), glyph.width, glyph.height,
                                  glyph.width, scale, scale, static_cast<int>(cp));
    }
    auto res = cache_.emplace(key, std::move(glyph));
    return &res.first->second;
}

double Font::measure(const std::string &text, double size)
{
    const int px = static_cast<int>(pixel_size(size) + 0.5);
    if (!loaded_ || px <= 0) {
        return 0.0;
    }
    double total = 0.0;
    for (unsigned cp : codepoints(text)) {
        const FontGlyph *glyph = glyph_for(cp, px);
        if (glyph) {
            total += glyph->advance;
        }
    }
    return total;
}

}  // namespace cb
