/* png.c — чтение и запись PNG на чистом C (через inflate.c, без zlib/libpng). */
#include "png.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inflate.h"

/* ---------------------------------------------------------------- утилиты */

static unsigned crc_table[256];
static int crc_ready = 0;

static void crc_init(void)
{
    unsigned c;
    int n, k;
    for (n = 0; n < 256; n++) {
        c = (unsigned)n;
        for (k = 0; k < 8; k++) {
            c = (c & 1) ? (0xedb88320u ^ (c >> 1)) : (c >> 1);
        }
        crc_table[n] = c;
    }
    crc_ready = 1;
}

static unsigned crc32_buf(const unsigned char *buf, size_t len)
{
    unsigned c = 0xffffffffu;
    size_t n;
    if (!crc_ready) {
        crc_init();
    }
    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c ^ 0xffffffffu;
}

static unsigned read_be32(const unsigned char *p)
{
    return ((unsigned)p[0] << 24) | ((unsigned)p[1] << 16) | ((unsigned)p[2] << 8) | p[3];
}

static void write_be32(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 24);
    p[1] = (unsigned char)(v >> 16);
    p[2] = (unsigned char)(v >> 8);
    p[3] = (unsigned char)v;
}

static int paeth(int a, int b, int c)
{
    int p = a + b - c;
    int pa = abs(p - a);
    int pb = abs(p - b);
    int pc = abs(p - c);
    if (pa <= pb && pa <= pc) {
        return a;
    }
    return (pb <= pc) ? b : c;
}

/* ---------------------------------------------------------------- декодер */

static int png_decode_rows(cb_image *img, unsigned char *raw, int bpp_raw,
                           int bitdepth, int colortype, const unsigned char *plte,
                           const unsigned char *trns)
{
    const int w = img->width;
    const int h = img->height;
    size_t stride = (size_t)w * (size_t)bpp_raw + 1;
    unsigned char *prev = NULL;
    int y;
    int x;

    for (y = 0; y < h; y++) {
        unsigned char *row = raw + (size_t)y * stride;
        int filter = row[0];
        unsigned char *cur = row + 1;
        size_t i;
        for (i = 0; i < (size_t)w * (size_t)bpp_raw; i++) {
            int a = (i >= (size_t)bpp_raw) ? cur[i - (size_t)bpp_raw] : 0;
            int b = prev ? prev[i] : 0;
            int c = (prev && i >= (size_t)bpp_raw) ? prev[i - (size_t)bpp_raw] : 0;
            switch (filter) {
                case 0: break;
                case 1: cur[i] = (unsigned char)(cur[i] + a); break;
                case 2: cur[i] = (unsigned char)(cur[i] + b); break;
                case 3: cur[i] = (unsigned char)(cur[i] + ((a + b) >> 1)); break;
                case 4: cur[i] = (unsigned char)(cur[i] + paeth(a, b, c)); break;
                default: return -1;
            }
        }
        /* распаковка в RGBA8 */
        for (x = 0; x < w; x++) {
            unsigned char *dst = img->pixels + ((size_t)y * (size_t)w + (size_t)x) * 4;
            switch (colortype) {
                case 0: {                              /* серый */
                    unsigned char g = cur[x * bpp_raw];
                    dst[0] = dst[1] = dst[2] = g;
                    dst[3] = 255;
                    break;
                }
                case 2: {                              /* RGB */
                    const unsigned char *p = cur + x * bpp_raw;
                    dst[0] = p[0];
                    dst[1] = p[1];
                    dst[2] = p[2];
                    dst[3] = 255;
                    break;
                }
                case 3: {                              /* палитра */
                    int idx = cur[x];
                    dst[0] = plte ? plte[idx * 3 + 0] : 0;
                    dst[1] = plte ? plte[idx * 3 + 1] : 0;
                    dst[2] = plte ? plte[idx * 3 + 2] : 0;
                    dst[3] = trns ? trns[idx] : 255;
                    break;
                }
                case 4: {                              /* серый + альфа */
                    const unsigned char *p = cur + x * bpp_raw;
                    dst[0] = dst[1] = dst[2] = p[0];
                    dst[3] = p[1];
                    break;
                }
                case 6: {                              /* RGBA */
                    const unsigned char *p = cur + x * bpp_raw;
                    dst[0] = p[0];
                    dst[1] = p[1];
                    dst[2] = p[2];
                    dst[3] = p[3];
                    break;
                }
                default:
                    return -1;
            }
        }
        prev = cur;
    }
    return 0;
}

int cb_png_decode(const unsigned char *data, size_t len, cb_image *out)
{
    static const unsigned char sig[8] = {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a};
    size_t pos = 8;
    int width = 0, height = 0, bitdepth = 0, colortype = 0, interlace = 0;
    unsigned char *idat = NULL;
    size_t idat_len = 0;
    unsigned char *plte = NULL;
    size_t plte_len = 0;
    unsigned char *trns = NULL;
    size_t trns_len = 0;
    int result = -1;

    if (len < 8 || memcmp(data, sig, 8) != 0) {
        return -1;
    }
    memset(out, 0, sizeof(*out));

    while (pos + 8 <= len) {
        unsigned clen = read_be32(data + pos);
        const char *type = (const char *)(data + pos + 4);
        const unsigned char *cdata = data + pos + 8;

        if (pos + 12 + clen > len) {
            break;
        }
        if (memcmp(type, "IHDR", 4) == 0 && clen >= 13) {
            width = (int)read_be32(cdata);
            height = (int)read_be32(cdata + 4);
            bitdepth = cdata[8];
            colortype = cdata[9];
            interlace = cdata[12];
        } else if (memcmp(type, "PLTE", 4) == 0) {
            plte = (unsigned char *)malloc(clen);
            if (!plte) goto done;
            memcpy(plte, cdata, clen);
            plte_len = clen;
        } else if (memcmp(type, "tRNS", 4) == 0) {
            trns = (unsigned char *)malloc(clen);
            if (!trns) goto done;
            memcpy(trns, cdata, clen);
            trns_len = clen;
        } else if (memcmp(type, "IDAT", 4) == 0) {
            unsigned char *nb = (unsigned char *)realloc(idat, idat_len + clen);
            if (!nb) goto done;
            idat = nb;
            memcpy(idat + idat_len, cdata, clen);
            idat_len += clen;
        } else if (memcmp(type, "IEND", 4) == 0) {
            break;
        }
        pos += 12 + clen;
    }

    if (!width || !height || !idat || bitdepth != 8 || interlace != 0) {
        goto done;
    }

    int channels;
    switch (colortype) {
        case 0: channels = 1; break;
        case 2: channels = 3; break;
        case 3: channels = 1; break;
        case 4: channels = 2; break;
        case 6: channels = 4; break;
        default: goto done;
    }

    size_t raw_len = (size_t)height * ((size_t)width * (size_t)channels + 1);
    unsigned char *raw = (unsigned char *)malloc(raw_len);
    if (!raw) goto done;
    size_t got = 0;
    if (cb_inflate_zlib(idat, idat_len, raw, raw_len, &got) != 0 || got < raw_len) {
        free(raw);
        goto done;
    }

    out->width = width;
    out->height = height;
    out->pixels = (unsigned char *)calloc((size_t)width * (size_t)height * 4, 1);
    if (!out->pixels) {
        free(raw);
        goto done;
    }
    if (png_decode_rows(out, raw, channels, bitdepth, colortype, plte, trns) != 0) {
        free(raw);
        cb_image_free(out);
        goto done;
    }
    free(raw);
    result = 0;

done:
    free(idat);
    free(plte);
    free(trns);
    (void)plte_len;
    (void)trns_len;
    return result;
}

int cb_png_load(const char *path, cb_image *out)
{
    FILE *fh = fopen(path, "rb");
    long size;
    unsigned char *buf;
    int rc;

    if (!fh) {
        return -1;
    }
    if (fseek(fh, 0, SEEK_END) != 0) {
        fclose(fh);
        return -1;
    }
    size = ftell(fh);
    if (size <= 0 || fseek(fh, 0, SEEK_SET) != 0) {
        fclose(fh);
        return -1;
    }
    buf = (unsigned char *)malloc((size_t)size);
    if (!buf) {
        fclose(fh);
        return -1;
    }
    if (fread(buf, 1, (size_t)size, fh) != (size_t)size) {
        free(buf);
        fclose(fh);
        return -1;
    }
    fclose(fh);
    rc = cb_png_decode(buf, (size_t)size, out);
    free(buf);
    return rc;
}

void cb_image_free(cb_image *img)
{
    if (img && img->pixels) {
        free(img->pixels);
        img->pixels = NULL;
        img->width = img->height = 0;
    }
}

/* ---------------------------------------------------------------- энкодер */

static int write_chunk(FILE *fh, const char *type, const unsigned char *data, unsigned len)
{
    unsigned char head[8];
    unsigned char crcbuf[4];
    unsigned char *tmp = (unsigned char *)malloc(len + 4);
    if (!tmp) {
        return -1;
    }
    write_be32(head, len);
    memcpy(head + 4, type, 4);
    memcpy(tmp, type, 4);
    if (len) {
        memcpy(tmp + 4, data, len);
    }
    write_be32(crcbuf, crc32_buf(tmp, len + 4));
    free(tmp);
    if (fwrite(head, 1, 8, fh) != 8) return -1;
    if (len && fwrite(data, 1, len, fh) != len) return -1;
    if (fwrite(crcbuf, 1, 4, fh) != 4) return -1;
    return 0;
}

int cb_png_save(const char *path, const cb_image *img)
{
    FILE *fh;
    unsigned char ihdr[13];
    unsigned char *raw = NULL;
    unsigned char *z = NULL;
    size_t raw_len = (size_t)img->height * ((size_t)img->width * 4 + 1);
    size_t zcap;
    size_t zlen;
    int rc = -1;
    int y;

    fh = fopen(path, "wb");
    if (!fh) {
        return -1;
    }
    raw = (unsigned char *)malloc(raw_len);
    if (!raw) {
        fclose(fh);
        return -1;
    }
    for (y = 0; y < img->height; y++) {
        raw[(size_t)y * ((size_t)img->width * 4 + 1)] = 0;       /* фильтр none */
        memcpy(raw + (size_t)y * ((size_t)img->width * 4 + 1) + 1,
               img->pixels + (size_t)y * (size_t)img->width * 4, (size_t)img->width * 4);
    }
    zcap = raw_len + raw_len / 65535 * 5 + 64;
    z = (unsigned char *)malloc(zcap);
    if (!z) {
        free(raw);
        fclose(fh);
        return -1;
    }
    zlen = cb_deflate_stored(raw, raw_len, z, zcap);
    if (!zlen) {
        goto done;
    }

    write_be32(ihdr, (unsigned)img->width);
    write_be32(ihdr + 4, (unsigned)img->height);
    ihdr[8] = 8;      /* бит на канал */
    ihdr[9] = 6;      /* RGBA */
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;

    if (fwrite("\x89PNG\r\n\x1a\n", 1, 8, fh) != 8) goto done;
    if (write_chunk(fh, "IHDR", ihdr, 13) != 0) goto done;
    if (write_chunk(fh, "IDAT", z, (unsigned)zlen) != 0) goto done;
    if (write_chunk(fh, "IEND", NULL, 0) != 0) goto done;
    rc = 0;

done:
    free(raw);
    free(z);
    fclose(fh);
    return rc;
}
