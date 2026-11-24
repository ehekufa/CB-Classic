/* png.h — минимальный декодер/энкодер PNG (RGBA8) на чистом C. */
#ifndef CB_PNG_H
#define CB_PNG_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int width;
    int height;
    unsigned char *pixels;      /* RGBA8, width*height*4 */
} cb_image;

int cb_png_decode(const unsigned char *data, size_t len, cb_image *out);
int cb_png_load(const char *path, cb_image *out);
int cb_png_save(const char *path, const cb_image *img);
void cb_image_free(cb_image *img);

#ifdef __cplusplus
}
#endif

#endif /* CB_PNG_H */
