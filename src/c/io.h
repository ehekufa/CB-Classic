/* io.h — маленькая обёртка над файловым вводом-выводом (C11). */
#ifndef CB_IO_H
#define CB_IO_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Читает файл целиком в буфер (malloc). Возвращает длину или -1. */
long cb_read_file(const char *path, unsigned char **out);

/* Пишет файл целиком. Возвращает 0 при успехе. */
int cb_write_file(const char *path, const void *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* CB_IO_H */
