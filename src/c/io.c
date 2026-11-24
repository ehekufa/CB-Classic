#include "io.h"

#include <stdio.h>
#include <stdlib.h>

long cb_read_file(const char *path, unsigned char **out)
{
    FILE *fh = fopen(path, "rb");
    long size;
    unsigned char *buf;

    *out = NULL;
    if (!fh) {
        return -1;
    }
    if (fseek(fh, 0, SEEK_END) != 0) {
        fclose(fh);
        return -1;
    }
    size = ftell(fh);
    if (size < 0 || fseek(fh, 0, SEEK_SET) != 0) {
        fclose(fh);
        return -1;
    }
    buf = (unsigned char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(fh);
        return -1;
    }
    if (size > 0 && fread(buf, 1, (size_t)size, fh) != (size_t)size) {
        free(buf);
        fclose(fh);
        return -1;
    }
    buf[size] = 0;
    fclose(fh);
    *out = buf;
    return size;
}

int cb_write_file(const char *path, const void *data, size_t len)
{
    FILE *fh = fopen(path, "wb");
    if (!fh) {
        return -1;
    }
    if (len && fwrite(data, 1, len, fh) != len) {
        fclose(fh);
        return -1;
    }
    fclose(fh);
    return 0;
}
