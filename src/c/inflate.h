/* inflate.h — DEFLATE/zlib без внешних зависимостей (см. inflate.c). */
#ifndef CB_INFLATE_H
#define CB_INFLATE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Распаковывает сырой DEFLATE-поток. Возвращает 0 при успехе, -1 при ошибке. */
int cb_inflate_raw(const unsigned char *in, size_t inlen,
                   unsigned char *out, size_t outcap, size_t *outlen);

/* Распаковывает zlib-поток (заголовок + adler32). */
int cb_inflate_zlib(const unsigned char *in, size_t inlen,
                    unsigned char *out, size_t outcap, size_t *outlen);

/* Упаковывает данные в zlib-поток несжатыми блоками. Возвращает размер или 0. */
size_t cb_deflate_stored(const unsigned char *in, size_t inlen,
                         unsigned char *out, size_t outcap);

unsigned cb_adler32(const unsigned char *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CB_INFLATE_H */
