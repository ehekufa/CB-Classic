/* Тест C-части: PNG-декодер/энкодер и inflate на реальных ассетах проекта. */
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "png.h"

static int files_ok = 0;
static int files_failed = 0;
static int jpeg_disguised = 0;

static void walk(const char *dir)
{
    DIR *d = opendir(dir);
    if (!d) {
        return;
    }
    struct dirent *entry;
    char path[1024];
    while ((entry = readdir(d)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
        struct stat st;
        if (stat(path, &st) != 0) {
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            walk(path);
            continue;
        }
        if (strstr(entry->d_name, ".png") == NULL) {
            continue;
        }
        cb_image img;
        if (cb_png_load(path, &img) != 0) {
            /* в проекте есть JPEG, переименованный в .png — это ожидаемо */
            FILE *fh = fopen(path, "rb");
            unsigned char head[3] = {0};
            if (fh) {
                size_t got = fread(head, 1, 3, fh);
                fclose(fh);
                if (got == 3 && head[0] == 0xff && head[1] == 0xd8 && head[2] == 0xff) {
                    jpeg_disguised++;
                    continue;
                }
            }
            printf("FAIL: %s\n", path);
            files_failed++;
            continue;
        }
        files_ok++;
        cb_image_free(&img);
    }
    closedir(d);
}

int main(int argc, char **argv)
{
    const char *root = argc > 1 ? argv[1] : "assets";
    walk(root);

    /* round-trip: кодирование + повторное чтение */
    cb_image img;
    if (cb_png_load("assets/icon.png", &img) != 0) {
        printf("FAIL: assets/icon.png\n");
        return 1;
    }
    if (cb_png_save("/tmp/cb_roundtrip.png", &img) != 0) {
        printf("FAIL: запись PNG\n");
        return 1;
    }
    cb_image back;
    if (cb_png_load("/tmp/cb_roundtrip.png", &back) != 0) {
        printf("FAIL: чтение записанного PNG\n");
        return 1;
    }
    long diff = 0;
    for (int i = 0; i < img.width * img.height * 4; i++) {
        if (img.pixels[i] != back.pixels[i]) {
            diff++;
        }
    }
    printf("PNG: прочитано %d, ошибок %d, JPEG-под-видом-PNG %d, round-trip отличий %ld\n",
           files_ok, files_failed, jpeg_disguised, diff);
    cb_image_free(&img);
    cb_image_free(&back);
    return files_failed == 0 && diff == 0 ? 0 : 1;
}
