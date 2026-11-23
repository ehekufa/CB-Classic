# Архитектура порта

Три языка разделены по ответственности: **C** — байты и картинки, **C++** — мир и кадр,
**Lua** — правила игры. Границы проходят по данным: Lua не знает про пиксели, C — про спрайты.

```
┌──────────────────────────────── assets/ ────────────────────────────────┐
│ project.lua  (сцены, объекты, картинки, звуки, переменные)              │
│ scenes/<сцена>/<объект>/images/*.png, sounds/*.mp3                      │
│ layout.lua   (стартовые позиции, восстановленные вручную)                │
└───────────────┬─────────────────────────────────────────────────────────┘
                │                     generated/lua/scenes/<NN>/obj_XXX.lua
                │                     (271 скрипт, сконвертированы из scripts.txt)
                ▼
        ┌──────────────── src/lua/boot.lua ────────────────┐
        │ 1. читает project.lua, описывает сцены движку     │
        │ 2. на старте сцены запускает триггеры             │
        │ 3. каждый кадр: касания → whenTheTruth → корутины │
        └───────┬───────────────────────────────┬───────────┘
                │ cb.*  (C-функции)             │ E.*  (рантайм блоков)
                ▼                               ▼
        ┌──── src/cpp/ ─────┐           src/lua/runtime/engine.lua
        │ Engine  (мир)     │           (переменные, арифметика Pocket Code,
        │ Renderer (кадр)   │            обёртки блоков, события)
        │ lua_api (мост)    │           src/lua/runtime/scheduler.lua
        └───────┬───────────┘           (корутины, wait/frame)
                │
                ▼
        ┌──── src/c/ ───────┐
        │ inflate.c  (DEFLATE, zlib-поток)   │
        │ png.c      (чтение/запись PNG RGBA)│
        │ io.c       (файлы)                 │
        └────────────────────┘
```

## Поток данных при старте

1. `main.cpp` → `App::init()` → `boot.start{scene, seed, project, profile}`.
2. `boot.start` читает `assets/project.lua` и вызывает для каждой сцены/объекта
   `cb.scene_define` / `cb.object_define` — движок получает описание мира.
3. `cb.finalize()` догружает размеры картинок (через `Engine::image` → `cb_png_load`),
   чтобы корректно считать размеры спрайтов (размер в Pocket Code — проценты от
   натуральной величины картинки).
4. `cb.scene_run(id)` создаёт экземпляры `Sprite` для объектов сцены.
5. `boot.activate(id)` подгружает Lua-модули объектов (`require "scenes.26.obj_021"`)
   и запускает все скрипты с триггером `start`.

## Кадр

```
App::run → step(dt):
    engine.advance(dt)                 # время
    boot.frame(dt):
        whenTheTruth: условие по фронту → новый поток
        back: кнопка «назад»
        scheduler:update(dt)           # корутины: wait/frame
        обновление текстовых полей (showVariable2)
        очередь нажатий (cb.poll_press): поле ввода получает фокус,
            иначе — скрипты trigger="tap"/"touch_down" сверху вниз
        подтверждённый ввод (cb.poll_text_commit) → переменная / E.ask
        отложенная смена сцены (E.run_scene → pending → activate)
    renderer.draw_scene(engine)        # программный рендер
    renderer.save_png(...)             # headless — в файл; SDL2 — в текстуру
```

## Координаты, размеры, слои

* Сцена — 1500×720: в `options.txt` проекта лежит разрешение устройства (720×1500) и
  `orientation: horizontal`, поэтому стороны меняются местами (см. `boot.start`).
* Начало координат — центр, X вправо, Y вверх, 1 единица = 1 пиксель сцены.
* `size` в `objects.txt` — доля (1.0 = 100%), в блоках `setSize` — проценты.
* Поворот: 0 = как в картинке, положительное значение — по часовой стрелке;
  `goSteps` двигает по направлению поворота (`x += steps·sin`, `y += steps·cos`).
* Слой (`setLayer`) сортирует отрисовку; при равенстве — по порядку создания.

## Мост Lua ↔ C++ (`cb.*`)

`src/cpp/lua_api.cpp` регистрирует таблицу `cb` (см. список `kApi`). Ключевое:

| группа | функции |
|---|---|
| проект | `scene_define`, `object_define`, `finalize`, `stage` |
| сцены | `scene_run`, `scene_name`, `scene_current`, `scene_count`, `scene_at` |
| спрайты | `obj_by_id`, `handles`, `get`, `set`, `set_image_id`, `next_image`, `go_steps`, `clone`, `delete`, `hit`, `hit_o` |
| ввод | `touch_down`, `touch_x/y`, `touch_count`, `touch_x/y_by_index`, `touch_id_by_index`, `poll_press`, `sprites_at` |
| текст | `text_create`, `text_set`, `text_delete`, `text_focus_at`, `text_focus`, `text_blur`, `text_focused`, `text_input`, `text_backspace`, `poll_text_commit` |
| звук | `play_sound`, `toast`, `bg_color` |
| хранилище | `firebase_set/get`, `store_set/get` (локальная эмуляция Firebase) |
| прочее | `log`, `time`, `dt`, `unsupported`, `exit` |

Свойства спрайта (`cb.get/handle, "поле"`): `x`, `y`, `rotation`, `size`, `alpha`,
`brightness`, `visible`, `layer`, `image_index`, `image_count`, `image_name`,
`sound_count`, `name`, `object_id`, `is_clone`, `width`, `height`.

## Ввод: нажатия и текст

```
SDL_FINGERDOWN(id) ──► Engine::touch_begin(id, x, y) ──► touches_ + очередь presses_
                                                            │
boot.frame ─► cb.poll_press() ─► cb.text_focus_at(x, y)==1 ? фокус поля
                                    : fire_touch_triggers(x, y)
                                          cb.sprites_at(x, y)  # сверху вниз
                                          первый спрайт со скриптами нажатия
```

* `Engine::sprites_at` проверяет **альфу пикселя картинки**: прозрачные места не ловят
  нажатие; если ни у кого нет непрозрачного пикселя в точке — работает проверка рамки
  (спрайты во весь экран пропускаются, иначе подложка съедала бы все нажатия).
* Касания хранятся списком: у каждого свой id от системы ввода и индекс 1..N в порядке
  нажатия — это то, что ожидают блоки `touchXByIndex` и `touchIDByIndex`.
* Фокус текстового поля: `SDL_START_TEXTINPUT` включает ввод (на Android открывается
  экранная клавиатура), `SDL_TEXTINPUT` добавляет UTF-8 символы, `Backspace` стирает
  последний кодпоинт, `Enter`/клик мимо подтверждает ввод: `Engine` складывает
  пару (метка, значение) в `text_commits_`, а `boot.lua` передаёт её в
  `E.on_text_commit` — тот пишет переменную (если поле к ней привязано) и вызывает
  обработчик `E.ask`.

## Рантайм блоков (`src/lua/runtime/engine.lua`)

Повторяет семантику Pocket Code, а не «похожее поведение»:

* `+` склеивает строки, если хотя бы один операнд не число; `-`, `×`, `÷` — арифметика;
* сравнения числовые, если оба операнда приводятся к числу, иначе строковые;
* пустая строка/`"false"`/`"0"` — ложь (`E.truthy`);
* переменные `globalVariable`/`localVariable` раздельные (глобальные — на проект,
  локальные — на экземпляр объекта, включая клоны);
* `saveVariable`/`readVariable` работают с локальным хранилищем (`saves/db.tsv`).

## Планировщик

Каждый скрипт — корутина. `E.wait(s)` возвращает `yield(s)`, `E.frame()` — `yield(0)`,
`while true` в `forever`/`repeatIsTrue` завершается `E.frame()`, чтобы поток не
блокировал кадр. Циклы `repeat N` тоже уступают кадр после итерации.

## Почему свой PNG и свой inflate

В песочнице нет `zlib.h`, `libpng`, SDL2 и Lua. Чтобы проект собирался «из коробки»:

* `src/c/inflate.c` — DEFLATE (stored/fixed/dynamic Huffman) + zlib-обёртка + adler32;
* `src/c/png.c` — чтение всех типов PNG (8 бит, палитра/серый/RGB/RGBA, фильтры 0–4)
  и запись RGBA8 (zlib-поток из stored-блоков);
* `third_party/lua-5.4` — исходники Lua 5.4.6 (MIT), собираются в статическую библиотеку;
* SDL2 — опциональный бэкенд (`make SDL2=1`), движок от него не зависит.

## Тесты

`tests/run_tests.sh` проверяет: сборку; декодирование всех 122 PNG и round-trip
кодирования; идемпотентность извлечения ассетов; 100% покрытие блоков конвертером;
синтаксис 116 Lua-модулей (`--check`); запуск сцен 42, 1, 26 с вводом; валидность
PNG-кадров и их размер.
