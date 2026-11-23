# minimp3

Один заголовочный файл MP3-декодера (CC0), нужен, чтобы играть звуки проекта
(`.mp3` в Pocket Code) без внешних библиотек.

- Источник: https://github.com/lieff/minimp3 (master, 2026-11)
- Файлы: `minimp3.h`, `LICENSE`
- Использование: `#define MINIMP3_IMPLEMENTATION` в одном .cpp (`src/cpp/audio.cpp`),
  в остальных — просто `#include "minimp3.h"`.
