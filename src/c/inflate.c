/* inflate.c — распаковщик DEFLATE/zlib без внешних зависимостей.
 *
 * Нужен, чтобы движок читал PNG из архива и писал кадры, не требуя zlib/libpng.
 * Реализация классическая: LSB-первый битовый поток, канонические коды Хаффмана
 * (stored / fixed / dynamic блоки), окно 32 КБ.
 */
#include "inflate.h"

#include <stdlib.h>
#include <string.h>

#define MAXBITS 15
#define MAXLCODES 286
#define MAXDCODES 30
#define FIXLCODES 288

typedef struct {
    const unsigned char *in;
    size_t inlen;
    size_t incnt;
    unsigned bitbuf;
    int bitcnt;
} bitreader;

typedef struct {
    short count[MAXBITS + 1];
    short symbol[FIXLCODES];
} huffman;

typedef struct {
    unsigned char *out;
    size_t outlen;
    size_t outcnt;
    int error;
} window;

static int br_bits(bitreader *br, int need)
{
    long val = br->bitbuf;
    while (br->bitcnt < need) {
        if (br->incnt == br->inlen) {
            return -1;
        }
        val |= (long)br->in[br->incnt++] << br->bitcnt;
        br->bitcnt += 8;
    }
    br->bitbuf = (unsigned)(val >> need);
    br->bitcnt -= need;
    return (int)(val & ((1L << need) - 1));
}

static int br_stored(bitreader *br, window *win)
{
    unsigned len, nlen;
    br->bitbuf = 0;
    br->bitcnt = 0;
    if (br->incnt + 4 > br->inlen) {
        return -1;
    }
    len = br->in[br->incnt] | ((unsigned)br->in[br->incnt + 1] << 8);
    nlen = br->in[br->incnt + 2] | ((unsigned)br->in[br->incnt + 3] << 8);
    br->incnt += 4;
    if (len != (~nlen & 0xffffu)) {
        return -1;
    }
    if (br->incnt + len > br->inlen || win->outcnt + len > win->outlen) {
        return -1;
    }
    memcpy(win->out + win->outcnt, br->in + br->incnt, len);
    win->outcnt += len;
    br->incnt += len;
    return 0;
}

static int huff_build(huffman *h, const short *length, int n)
{
    int symbol;
    int len;
    int left;
    short offs[MAXBITS + 1];

    for (len = 0; len <= MAXBITS; len++) {
        h->count[len] = 0;
    }
    for (symbol = 0; symbol < n; symbol++) {
        h->count[length[symbol]]++;
    }
    if (h->count[0] == n) {
        return 0;                                 /* пустое дерево допускается */
    }
    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= h->count[len];
        if (left < 0) {
            return left;                          /* переподписка */
        }
    }
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++) {
        offs[len + 1] = (short)(offs[len] + h->count[len]);
    }
    for (symbol = 0; symbol < n; symbol++) {
        if (length[symbol] != 0) {
            h->symbol[offs[length[symbol]]++] = (short)symbol;
        }
    }
    return left;
}

static int huff_decode(bitreader *br, const huffman *h)
{
    int code = 0;
    int first = 0;
    int index = 0;
    int len;
    for (len = 1; len <= MAXBITS; len++) {
        int bit = br_bits(br, 1);
        if (bit < 0) {
            return -1;
        }
        code |= bit;
        int count = h->count[len];
        if (code - count < first) {
            return h->symbol[index + (code - first)];
        }
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -1;
}

static int inflate_codes(bitreader *br, window *win, const huffman *lencode, const huffman *distcode)
{
    static const short lbase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                                    35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                                   3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dbase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                                    257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
                                    8193, 12289, 16385, 24577};
    static const short dext[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                                   7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    for (;;) {
        int symbol = huff_decode(br, lencode);
        if (symbol < 0) {
            return -1;
        }
        if (symbol < 256) {
            if (win->outcnt >= win->outlen) {
                return -1;
            }
            win->out[win->outcnt++] = (unsigned char)symbol;
        } else if (symbol == 256) {
            return 0;
        } else {
            symbol -= 257;
            if (symbol >= 29) {
                return -1;
            }
            int len = lbase[symbol] + br_bits(br, lext[symbol]);
            int dsym = huff_decode(br, distcode);
            if (dsym < 0 || dsym >= 30) {
                return -1;
            }
            int dist = dbase[dsym] + br_bits(br, dext[dsym]);
            if ((size_t)dist > win->outcnt) {
                return -1;
            }
            if (win->outcnt + (size_t)len > win->outlen) {
                return -1;
            }
            while (len--) {
                win->out[win->outcnt] = win->out[win->outcnt - (size_t)dist];
                win->outcnt++;
            }
        }
    }
}

static int inflate_fixed(bitreader *br, window *win)
{
    static huffman lencode, distcode;
    static int built = 0;
    if (!built) {
        short lengths[FIXLCODES];
        int symbol;
        for (symbol = 0; symbol < 144; symbol++) lengths[symbol] = 8;
        for (; symbol < 256; symbol++) lengths[symbol] = 9;
        for (; symbol < 280; symbol++) lengths[symbol] = 7;
        for (; symbol < FIXLCODES; symbol++) lengths[symbol] = 8;
        huff_build(&lencode, lengths, FIXLCODES);
        for (symbol = 0; symbol < MAXDCODES; symbol++) lengths[symbol] = 5;
        huff_build(&distcode, lengths, MAXDCODES);
        built = 1;
    }
    return inflate_codes(br, win, &lencode, &distcode);
}

static int inflate_dynamic(bitreader *br, window *win)
{
    static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    short lengths[MAXLCODES + MAXDCODES];
    huffman lencode, distcode;
    int nlen, ndist, ncode, index, err;

    nlen = br_bits(br, 5) + 257;
    ndist = br_bits(br, 5) + 1;
    ncode = br_bits(br, 4) + 4;
    if (nlen > MAXLCODES || ndist > MAXDCODES) {
        return -1;
    }
    for (index = 0; index < ncode; index++) {
        lengths[order[index]] = (short)br_bits(br, 3);
    }
    for (; index < 19; index++) {
        lengths[order[index]] = 0;
    }
    if (huff_build(&lencode, lengths, 19) != 0) {
        return -1;
    }

    index = 0;
    while (index < nlen + ndist) {
        int symbol = huff_decode(br, &lencode);
        if (symbol < 0) {
            return -1;
        }
        if (symbol < 16) {
            lengths[index++] = (short)symbol;
        } else {
            int len = 0;
            int repeat;
            if (symbol == 16) {
                if (index == 0) {
                    return -1;
                }
                len = lengths[index - 1];
                repeat = 3 + br_bits(br, 2);
            } else if (symbol == 17) {
                repeat = 3 + br_bits(br, 3);
            } else {
                repeat = 11 + br_bits(br, 7);
            }
            if (index + repeat > nlen + ndist) {
                return -1;
            }
            while (repeat--) {
                lengths[index++] = (short)len;
            }
        }
    }

    err = huff_build(&lencode, lengths, nlen);
    if (err != 0 && (err < 0 || nlen != lencode.count[0] + lencode.count[1])) {
        return -1;
    }
    err = huff_build(&distcode, lengths + nlen, ndist);
    if (err != 0 && (err < 0 || ndist != distcode.count[0] + distcode.count[1])) {
        return -1;
    }
    return inflate_codes(br, win, &lencode, &distcode);
}

int cb_inflate_raw(const unsigned char *in, size_t inlen,
                   unsigned char *out, size_t outcap, size_t *outlen)
{
    bitreader br;
    window win;
    int last, type, status;

    br.in = in;
    br.inlen = inlen;
    br.incnt = 0;
    br.bitbuf = 0;
    br.bitcnt = 0;
    win.out = out;
    win.outlen = outcap;
    win.outcnt = 0;
    win.error = 0;

    do {
        last = br_bits(&br, 1);
        type = br_bits(&br, 2);
        if (last < 0 || type < 0) {
            return -1;
        }
        if (type == 0) {
            status = br_stored(&br, &win);
        } else if (type == 1) {
            status = inflate_fixed(&br, &win);
        } else if (type == 2) {
            status = inflate_dynamic(&br, &win);
        } else {
            status = -1;
        }
        if (status != 0) {
            return -1;
        }
    } while (!last);

    if (outlen) {
        *outlen = win.outcnt;
    }
    return 0;
}

unsigned cb_adler32(const unsigned char *data, size_t len)
{
    unsigned a = 1, b = 0;
    size_t i;
    for (i = 0; i < len; i++) {
        a = (a + data[i]) % 65521u;
        b = (b + a) % 65521u;
    }
    return (b << 16) | a;
}

int cb_inflate_zlib(const unsigned char *in, size_t inlen,
                    unsigned char *out, size_t outcap, size_t *outlen)
{
    if (inlen < 6) {                              /* 2 байта заголовка + adler32 */
        return -1;
    }
    if ((in[0] & 0x0f) != 8) {
        return -1;
    }
    if (((in[0] << 8) | in[1]) % 31 != 0) {
        return -1;
    }
    if (in[1] & 0x20) {                           /* preset dictionary не поддерживаем */
        return -1;
    }
    return cb_inflate_raw(in + 2, inlen - 2, out, outcap, outlen);
}

size_t cb_deflate_stored(const unsigned char *in, size_t inlen,
                         unsigned char *out, size_t outcap)
{
    /* zlib-поток из несжатых (stored) deflate-блоков: проще и достаточно для кадров */
    size_t o = 0;
    size_t pos = 0;

    if (outcap < 2 + 5 + inlen + (inlen / 65535 + 1) * 5 + 4) {
        return 0;
    }
    out[o++] = 0x78;
    out[o++] = 0x01;
    do {
        size_t chunk = inlen - pos;
        int final;
        if (chunk > 65535) {
            chunk = 65535;
        }
        final = (pos + chunk == inlen);
        out[o++] = (unsigned char)(final ? 1 : 0);
        out[o++] = (unsigned char)(chunk & 0xff);
        out[o++] = (unsigned char)((chunk >> 8) & 0xff);
        out[o++] = (unsigned char)(~chunk & 0xff);
        out[o++] = (unsigned char)((~chunk >> 8) & 0xff);
        memcpy(out + o, in + pos, chunk);
        o += chunk;
        pos += chunk;
    } while (pos < inlen);

    unsigned adler = cb_adler32(in, inlen);
    out[o++] = (unsigned char)((adler >> 24) & 0xff);
    out[o++] = (unsigned char)((adler >> 16) & 0xff);
    out[o++] = (unsigned char)((adler >> 8) & 0xff);
    out[o++] = (unsigned char)(adler & 0xff);
    return o;
}
