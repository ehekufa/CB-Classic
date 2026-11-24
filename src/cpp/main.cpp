// main.cpp — точка входа: headless-прогон и (опционально) окно через SDL2.
//
// Примеры:
//   ./cb-classic --frames 90 --scene 1 --out out/frames
//   ./cb-classic --script tests/scripts/click_menu.txt --frames 200 --dump-state
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <vector>

#include <sys/stat.h>
#include <sys/types.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "app.hpp"
#ifdef CB_USE_SDL2
#include "view_sdl.hpp"
#endif

#ifdef CB_USE_SDL2
#include <SDL2/SDL.h>
#ifdef __ANDROID__
#include <SDL2/SDL_system.h>
#endif
#endif

namespace {

void mkdir_p(const std::string &path)
{
    std::string current;
    for (size_t i = 0; i < path.size(); ++i) {
        current.push_back(path[i]);
        if (path[i] == '/' || i + 1 == path.size()) {
            if (current != "/") {
                mkdir(current.c_str(), 0755);
            }
        }
    }
}

std::vector<std::string> read_lines(const std::string &path)
{
    std::vector<std::string> lines;
    std::ifstream in(path);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line[0] != '#') {
            lines.push_back(line);
        }
    }
    return lines;
}

}  // namespace

namespace cb {

App::App(AppOptions opts)
    : engine(opts.root + "/assets"), renderer(1, 1), options(std::move(opts)) {}

App::~App() { close_lua(); }

bool App::open_lua()
{
    lua = luaL_newstate();
    if (!lua) {
        std::fprintf(stderr, "не удалось создать состояние Lua\n");
        return false;
    }
    luaL_openlibs(lua);
    register_api(lua, this);

    std::string lua_path = options.root + "/src/lua/?.lua;" +
                           options.root + "/generated/lua/?.lua;" +
                           options.root + "/generated/lua/?/init.lua;./?.lua";
    lua_getglobal(lua, "package");
    lua_pushstring(lua, lua_path.c_str());
    lua_setfield(lua, -2, "path");
    lua_pop(lua, 1);
    return true;
}

void App::close_lua()
{
    if (lua) {
        lua_close(lua);
        lua = nullptr;
    }
}

bool App::init()
{
    if (!open_lua()) {
        return false;
    }

    // 0) сеть включаем ДО первых скриптов: в новой версии сцена «Загрузка»
    // сразу пишет в базу (ScreenWidth/ScreenHeight) и читает прогресс, так что
    // включение после boot теряло самые первые записи.
    if (!options.fb_offline) {
        if (!options.fb_url.empty()) {
            engine.fb_configure(options.fb_url);
        }
        if (!options.fb_url.empty() || options.fb_auto) {
            if (engine.fb_base_url().empty()) {
                engine.fb_configure(cb::FbNet::default_url());
            }
            engine.fb_network(true);
        }
    }

    // 1) грузим проект
    if (luaL_dostring(lua,
            "boot = require('boot')") != LUA_OK) {
        std::fprintf(stderr, "ошибка загрузки boot.lua: %s\n", lua_tostring(lua, -1));
        return false;
    }
    lua_getglobal(lua, "boot");
    lua_getfield(lua, -1, "start");
    lua_newtable(lua);
    if (options.scene > 0) {
        lua_pushinteger(lua, options.scene);
        lua_setfield(lua, -2, "scene");
    }
    lua_pushinteger(lua, static_cast<lua_Integer>(options.seed));
    lua_setfield(lua, -2, "seed");
    std::string project_path = options.root + "/assets/project.lua";
    lua_pushstring(lua, project_path.c_str());
    lua_setfield(lua, -2, "project");

    if (!options.profile.empty()) {
        lua_newtable(lua);
        std::string item;
        std::istringstream stream(options.profile);
        while (std::getline(stream, item, ',')) {
            size_t eq = item.find('=');
            if (eq == std::string::npos) {
                continue;
            }
            std::string key = item.substr(0, eq);
            std::string value = item.substr(eq + 1);
            lua_pushstring(lua, value.c_str());
            lua_setfield(lua, -2, key.c_str());
        }
        lua_setfield(lua, -2, "profile");
    }
    if (lua_pcall(lua, 1, 1, 0) != LUA_OK) {
        std::fprintf(stderr, "ошибка старта проекта: %s\n", lua_tostring(lua, -1));
        return false;
    }
    bool ok = lua_toboolean(lua, -1) != 0;
    lua_pop(lua, 1);

    // 2) сеть: адрес базы пришёл из игры (E.FB_URL), CLI может его перебить.
    // Включаем её на телефоне всегда, на десктопе — только по --fb-url/--fb-on,
    // иначе headless-тесты ходили бы в интернет.
    // адрес из CLI главнее адреса, который прописала игра (E.FB_URL)
    if (!options.fb_offline && !options.fb_url.empty()) {
        engine.fb_configure(options.fb_url);
    }
    engine.log(std::string("сеть: ") + (engine.fb_online() ? "включена, база " + engine.fb_base_url()
                                                           : std::string("выключена")));

    if (options.width <= 0) {
        options.width = engine.stage_width();
    }
    if (options.height <= 0) {
        options.height = engine.stage_height();
    }
    renderer = Renderer(options.width, options.height, options.pixel_scale);
    // Шрифт. В архиве Pocket Code шрифтов нет: оригинал рисовал текст системным
    // шрифтом Android (Roboto), поэтому на телефоне берём его прямо с устройства.
    // Запасной путь — DejaVu Sans, упакованный в APK (MainActivity кладёт его
    // в filesDir/root/fonts): он же используется в headless-прогонах и тестах.
    std::vector<std::string> font_candidates;
    // Roboto — тот самый шрифт, которым рисовал текст Pocket Code на Android.
    font_candidates.push_back(options.root + "/fonts/Roboto-Regular.ttf");
    font_candidates.push_back(options.root + "/third_party/fonts/Roboto-Regular.ttf");
#ifdef __ANDROID__
    font_candidates.push_back("/system/fonts/Roboto-Regular.ttf");
    font_candidates.push_back("/system/fonts/RobotoCondensed-Regular.ttf");
    font_candidates.push_back("/system/fonts/NotoSans-Regular.ttf");
    font_candidates.push_back("/system/fonts/DroidSans.ttf");
#endif
    font_candidates.push_back(options.root + "/fonts/DejaVuSans.ttf");
    font_candidates.push_back(options.root + "/third_party/fonts/DejaVuSans.ttf");
    renderer.load_font(font_candidates);
    // Звук: эффекты проекта (mp3). В headless-сборке
    // без SDL звук только попадает в журнал, но файлы всё равно декодируются.
    if (!options.no_sound) {
        engine.set_sound_hook([this](const std::string &name, bool loop) {
            if (name.empty()) {
                return;
            }
            std::string path = name.rfind('/', 0) == 0 || name.find(':') != std::string::npos
                                   ? name
                                   : engine.asset_root() + "/" + name;
            if (loop) {
                audio.play_looped(path, 0.8);
            } else {
                audio.play(path, 0.9);
            }
        });
    }
    if (renderer.has_font()) {
        engine.log("шрифт: " + renderer.font_path());
        engine.set_text_measurer([this](const std::string &text, double size) {
            return renderer.text_width(text, size);
        });
    } else {
        engine.log("шрифт не найден — встроенный 5x7");
    }
    return ok;
}

bool App::open_audio()
{
    if (options.no_sound) {
        return false;
    }
    bool ok = audio.open();
    engine.log(std::string("звук: ") +
               (ok ? "устройство открыто" : "только журнал (нет SDL)"));
    return ok;
}

void App::step(double dt)
{
    // отложенные переменные из --set-var (проверки в тестах)
    for (auto &entry : options.set_vars) {
        if (entry.frame != frame_index_) {
            continue;
        }
        lua_getglobal(lua, "boot");
        lua_getfield(lua, -1, "debug_set_var");
        if (lua_isfunction(lua, -1)) {
            lua_pushinteger(lua, entry.id);
            lua_pushstring(lua, entry.value.c_str());
            if (lua_pcall(lua, 2, 1, 0) == LUA_OK) {
                engine.log("set-var: " + std::to_string(entry.id) + "=" + entry.value);
            }
            lua_pop(lua, 1);
        } else {
            lua_pop(lua, 1);
        }
        lua_pop(lua, 1);
    }
    ++frame_index_;
    engine.advance(dt);
    lua_getglobal(lua, "boot");
    lua_getfield(lua, -1, "frame");
    lua_pushnumber(lua, dt);
    if (lua_pcall(lua, 1, 0, 0) != LUA_OK) {
        engine.log(std::string("ошибка кадра: ") + lua_tostring(lua, -1));
        lua_pop(lua, 1);
    }
    lua_pop(lua, 1);
}

// --trace ID: печатает состояние одного объекта (по object_id) каждый кадр —
// помогает разбирать баги «кнопка не отжалась», «объект остался серым» и т.п.
void App::trace_object(int object_id, int frame)
{
    cb::Sprite *sprite = engine.find_by_object(object_id);
    if (!sprite) {
        return;
    }
    std::printf("trace %4d сцена=%2d id=%-3d x=%7.1f y=%7.1f vis=%d ярк=%5.1f "
                "прозр=%5.1f размер=%5.1f картинка=%s\n",
                frame, engine.current_scene_id(), object_id, sprite->x, sprite->y,
                sprite->visible ? 1 : 0, sprite->brightness, sprite->alpha, sprite->size,
                sprite->current_image() ? sprite->current_image()->path.c_str() : "-");
}

void App::render_to_disk(const std::string &path)
{
    renderer.draw_scene(engine);
    if (!renderer.save_png(path)) {
        std::fprintf(stderr, "не удалось записать %s\n", path.c_str());
    }
}

void App::apply_input_command(const std::string &line)
{
    std::istringstream in(line);
    std::string cmd;
    in >> cmd;
    if (cmd == "press") {
        double x = 0, y = 0;
        in >> x >> y;
        engine.push_touch_press(x, y);
    } else if (cmd == "release") {
        engine.set_touch(false, engine.touch_x(), engine.touch_y(), 0);
    } else if (cmd == "move") {
        // move [id] x y: без id двигаем последний палец, иначе появлялся
        // «фантомный» нулевой палец и джойстик считал касания неверно
        std::vector<double> numbers;
        double value = 0.0;
        while (in >> value) {
            numbers.push_back(value);
        }
        double x = 0.0;
        double y = 0.0;
        int id = -1;
        if (numbers.size() >= 3) {
            id = static_cast<int>(numbers[0]);
            x = numbers[1];
            y = numbers[2];
        } else if (numbers.size() == 2) {
            x = numbers[0];
            y = numbers[1];
        }
        if (id >= 0) {
            engine.touch_move(id, x, y);
        } else if (engine.touch_count() > 0) {
            engine.touch_move(engine.last_touch_id(), x, y);
        } else {
            engine.set_touch(true, x, y, 1);
        }
    } else if (cmd == "type") {
        std::string text = line.substr(line.find(cmd) + cmd.size());
        size_t start = text.find_first_not_of(' ');
        std::string payload = start == std::string::npos ? "" : text.substr(start);
        engine.text_input(payload);
    } else if (cmd == "key") {
        std::string key;
        in >> key;
        if (key == "backspace") {
            engine.text_backspace();
        } else if (key == "enter") {
            engine.blur_text();
        } else if (key == "esc") {
            if (engine.focused_text().empty()) {
                apply_input_command("back");
            } else {
                engine.blur_text();
            }
        }
    } else if (cmd == "touch") {
        int id = 0;
        double x = 0, y = 0;
        in >> id >> x >> y;
        engine.touch_begin(id, x, y);
    } else if (cmd == "lift") {
        int id = 0;
        in >> id;
        engine.touch_end(id);
    } else if (cmd == "tap") {
        double x = 0, y = 0;
        in >> x >> y;
        engine.touch_begin(0, x, y);
        engine.touch_end(0);
    } else if (cmd == "back") {
        lua_getglobal(lua, "boot");
        lua_getfield(lua, -1, "back");
        lua_pcall(lua, 0, 0, 0);
        lua_pop(lua, 1);
    } else if (cmd == "scene") {
        int id = 0;
        in >> id;
        lua_getglobal(lua, "boot");
        lua_getfield(lua, -1, "run_scene");
        lua_pushinteger(lua, id);
        lua_pcall(lua, 1, 0, 0);
        lua_pop(lua, 1);
    }
}

bool App::check_buttons()
{
    int total = 0;
    int broken = 0;
    int hidden = 0;
    for (const cb::SceneDef &scene : engine.scenes()) {
        lua_getglobal(lua, "boot");
        lua_getfield(lua, -1, "run_scene");
        lua_pushinteger(lua, scene.id);
        if (lua_pcall(lua, 1, 0, 0) != LUA_OK) {
            std::fprintf(stderr, "сцена %d: %s\n", scene.id, lua_tostring(lua, -1));
            lua_pop(lua, 2);
            continue;
        }
        lua_pop(lua, 1);                        // остаётся таблица boot
        for (int i = 0; i < 3; ++i) {           // дать отработать start-скриптам
            step(1.0 / 60.0);
        }
        lua_getglobal(lua, "boot");
        lua_getfield(lua, -1, "check_buttons");
        if (lua_pcall(lua, 0, 1, 0) != LUA_OK) {
            std::fprintf(stderr, "сцена %d: %s\n", scene.id, lua_tostring(lua, -1));
            lua_pop(lua, 2);
            continue;
        }
        lua_getfield(lua, -1, "buttons");
        lua_Integer count = luaL_len(lua, -1);
        std::printf("\nсцена %d «%s»: кнопок %d\n", scene.id, scene.name.c_str(),
                    static_cast<int>(count));
        for (lua_Integer i = 1; i <= count; ++i) {
            lua_rawgeti(lua, -1, i);
            lua_getfield(lua, -1, "name");
            lua_getfield(lua, -2, "reached");
            lua_getfield(lua, -3, "blocker");
            lua_getfield(lua, -4, "invisible");
            const char *name = lua_tostring(lua, -4);
            bool reached = lua_toboolean(lua, -3) != 0;
            const char *blocker = lua_tostring(lua, -2);
            bool invisible = lua_toboolean(lua, -1) != 0;
            total++;
            if (invisible) {
                hidden++;
                std::printf("  ---  %-28s нет картинки (невидим и в оригинале)\n",
                            name ? name : "?");
            } else if (reached) {
                std::printf("  ok   %-28s\n", name ? name : "?");
            } else {
                broken++;
                std::printf("  НЕТ  %-28s перекрыт: %s\n", name ? name : "?",
                            blocker ? blocker : "прозрачность/рамка");
            }
            lua_pop(lua, 5);                    // entry, name, reached, blocker, invisible
        }
        lua_pop(lua, 3);                    // buttons, результат, boot
    }
    std::printf("\nвсего кнопок: %d, не получают нажатие: %d, без картинки: %d\n",
                total, broken, hidden);
    return broken == 0;
}

bool App::run()
{
    mkdir_p(options.out_dir);
    auto script = options.script.empty() ? std::vector<std::string>()
                                         : read_lines(options.script);
    size_t script_pos = 0;
    double dt = 1.0 / options.fps;
    int frames = options.frames > 0 ? options.frames : 1;
    if (options.bench > 0) {
        frames = options.bench;
    }
    // --bench: время кадра без записи PNG (на телефоне именно отрисовка
    // определяет плавность). Считаем отдельно Lua/шаг и растеризацию.
    using Clock = std::chrono::steady_clock;
    double bench_step_ms = 0.0;
    double bench_draw_ms = 0.0;
    double bench_worst_ms = 0.0;

    for (int frame = 0; frame < frames; ++frame) {
        if (engine.exit_requested()) {
            break;
        }
        // сценарий ввода: одна строка на кадр, префикс "f<N>:" — прыжок к кадру
        while (script_pos < script.size()) {
            const std::string &line = script[script_pos];
            if (line[0] == 'f') {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    int target = std::atoi(line.c_str() + 1);
                    if (frame < target) {
                        break;
                    }
                    apply_input_command(line.substr(colon + 1));
                    ++script_pos;
                    continue;
                }
            }
            apply_input_command(line);
            ++script_pos;
        }
        auto t0 = Clock::now();
        step(dt);
        if (!options.trace_objects.empty()) {
            for (int id : options.trace_objects) {
                trace_object(id, frame);
            }
        } else if (options.trace_object >= 0) {
            trace_object(options.trace_object, frame);
        }
        auto t1 = Clock::now();
        if (options.bench > 0) {
            renderer.draw_scene(engine);
        } else {
            char name[256];
            std::snprintf(name, sizeof(name), "%s/frame_%04d.png", options.out_dir.c_str(), frame);
            render_to_disk(name);
        }
        auto t2 = Clock::now();
        if (options.bench > 0) {
            double step_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
            double draw_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
            bench_step_ms += step_ms;
            bench_draw_ms += draw_ms;
            bench_worst_ms = std::max(bench_worst_ms, step_ms + draw_ms);
        }
        if (options.realtime) {
            const auto deadline = t0 + std::chrono::duration_cast<Clock::duration>(
                                               std::chrono::duration<double>(dt));
            std::this_thread::sleep_until(deadline);
        }
        if (options.verbose && frame < 8) {
            std::fprintf(stderr, "[кадр %d] сцена=%d журнал=%zu спрайтов=%zu\n", frame,
                         engine.current_scene_id(), engine.log_lines().size(),
                         engine.sprites_in_draw_order().size());
        }
        if (options.verbose && frame % 30 == 0) {
            std::string fb;
            for (const auto &pair : engine.fb_all()) {
                fb += pair.first + "=" + pair.second + " ";
            }
            std::printf("  база: %s\n", fb.c_str());
            std::printf("кадр %d: сцена %d, спрайтов %d\n", frame,
                        engine.current_scene_id(), renderer.drawn_sprites());
        }
    }

    std::printf("сцена: %d (%s), кадров: %d\n", engine.current_scene_id(),
                engine.current_scene_name().c_str(), frames);
    std::printf("спрайтов на сцене: %zu, записей журнала: %zu\n",
                engine.sprites_in_draw_order().size(), engine.log_lines().size());
    std::printf("прозвучало звуков: %zu\n", engine.sound_log().size());
    if (options.bench > 0 && frames > 0) {
        std::printf("бенч: %d кадров, буфер %dx%d (x%.2f)\n", frames, renderer.width(),
                    renderer.height(), renderer.scale());
        double total_ms = bench_step_ms + bench_draw_ms;
        double fps = total_ms > 0.0 ? 1000.0 * frames / total_ms : 0.0;
        std::printf("бенч: шаг Lua %.2f мс, отрисовка %.2f мс, кадр %.2f мс (макс %.2f) -> %.1f fps\n",
                    bench_step_ms / frames, bench_draw_ms / frames, total_ms / frames,
                    bench_worst_ms, fps);
    }
    lua_getglobal(lua, "boot");
    lua_getfield(lua, -1, "stats");
    if (lua_pcall(lua, 0, 1, 0) == LUA_OK && lua_istable(lua, -1)) {
        lua_getfield(lua, -1, "frames");
        int frames_done = static_cast<int>(lua_tointeger(lua, -1));
        lua_pop(lua, 1);
        lua_getfield(lua, -1, "spawns");
        int spawns = static_cast<int>(lua_tointeger(lua, -1));
        lua_pop(lua, 1);
        lua_getfield(lua, -1, "threads");
        int threads = static_cast<int>(lua_tointeger(lua, -1));
        lua_pop(lua, 1);
        lua_getfield(lua, -1, "errors");
        int errors = static_cast<int>(lua_tointeger(lua, -1));
        lua_pop(lua, 1);
        std::printf("кадров обработано: %d, запусков скриптов: %d, живых потоков: %d, "
                    "ошибок: %d\n", frames_done, spawns, threads, errors);
    }
    lua_pop(lua, 2);
    if (options.verbose) {
        std::printf("фон: #%08X, размер сцены: %dx%d\n", engine.background_rgba(),
                    engine.stage_width(), engine.stage_height());
        std::printf("\n%-6s %-22s %8s %8s %7s %7s %6s %4s %-30s\n",
                    "handle", "имя", "x", "y", "size", "alpha", "ярк", "vis", "картинка");
        for (cb::Sprite *sprite : engine.sprites_in_draw_order()) {
            const cb::ImageRef *ref = sprite->current_image();
            std::printf("%-6d %-22s %8.1f %8.1f %7.1f %7.1f %6.1f %4d %s (%dx%d)\n",
                        sprite->handle, sprite->name.c_str(), sprite->x, sprite->y,
                        sprite->size, sprite->alpha, sprite->brightness,
                        sprite->visible ? 1 : 0,
                        ref ? ref->path.c_str() : "-", ref ? ref->width : 0,
                        ref ? ref->height : 0);
        }
        std::printf("\nтекстовые поля:\n");
        for (const auto &pair : engine.texts()) {
            const cb::TextField &field = pair.second;
            std::printf("  %-16s pos=%.0f,%.0f size=%.0f align=%-6s видно=%d значение=\"%s\" "
                        "placeholder=\"%s\"\n", field.label.c_str(), field.x, field.y,
                        field.size, field.align.c_str(), field.visible ? 1 : 0,
                        field.value.c_str(), field.placeholder.c_str());
        }
        std::printf("\nжурнал:\n");
        for (const auto &line : engine.log_lines()) {
            std::printf("  %s\n", line.c_str());
        }
    }
    return true;
}

void App::shutdown() { close_lua(); }

}  // namespace cb

#ifdef __ANDROID__
// На Android ассеты распаковываются Java-кодом в filesDir/root (см. MainActivity.java).
static std::string android_asset_root()
{
    const char *base = SDL_AndroidGetInternalStoragePath();
    return std::string(base ? base : ".") + "/root";
}
#endif

int main(int argc, char **argv)
{
    cb::AppOptions opts;
#ifdef __ANDROID__
    opts.root = android_asset_root();
    opts.scene = 42;                 // старт с загрузочной сцены, как в оригинале
    opts.headless = false;
    opts.fps = 60.0;
    opts.fb_auto = true;             // на телефоне база нужна всегда
#endif
    std::vector<std::string> args(argv + 1, argv + argc);
    bool seed_given = false;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];
        auto next = [&](const char *def) -> std::string {
            return (i + 1 < args.size()) ? args[++i] : def;
        };
        if (a == "--seed") {
            // только для тестов: в игре зерно берётся из часов, иначе каждый бой
            // выпадал бы один и тот же скин врага и один и тот же лут
            opts.seed = static_cast<unsigned>(std::strtoul(next("0").c_str(), nullptr, 10));
            seed_given = true;
        } else if (a == "--root") {
            opts.root = next(".");
        } else if (a == "--scene") {
            opts.scene = std::atoi(next("0").c_str());
        } else if (a == "--frames") {
            opts.frames = std::atoi(next("1").c_str());
        } else if (a == "--fps") {
            opts.fps = std::atof(next("30").c_str());
        } else if (a == "--out") {
            opts.out_dir = next("out");
        } else if (a == "--script") {
            opts.script = next("");
        } else if (a == "--size") {
            std::string s = next("720x1348");
            size_t x = s.find('x');
            if (x != std::string::npos) {
                opts.width = std::atoi(s.substr(0, x).c_str());
                opts.height = std::atoi(s.substr(x + 1).c_str());
            }
        } else if (a == "--no-sound") {
            opts.no_sound = true;
        } else if (a == "--window") {
            opts.headless = false;
        } else if (a == "--window-frames") {
            opts.window_frames = std::atoi(next("60").c_str());
        } else if (a == "--screenshot") {
            opts.screenshot = next("shot.png");
        } else if (a == "--bench") {
            opts.bench = std::atoi(next("60").c_str());
        } else if (a == "--pixel-scale") {
            opts.pixel_scale = std::atof(next("2").c_str());
            if (opts.pixel_scale < 1.0) {
                opts.pixel_scale = 1.0;
            }
        } else if (a == "--check-buttons") {
            opts.check_buttons = true;
        } else if (a == "--check") {
            opts.check_dir = next("generated/lua");
        } else if (a == "--set-var") {
            std::string spec = next("4=0@0");       // id=значение[@кадр]
            int frame_no = 0;
            size_t at = spec.find('@');
            if (at != std::string::npos) {
                frame_no = std::atoi(spec.c_str() + at + 1);
                spec = spec.substr(0, at);
            }
            size_t eq = spec.find('=');
            if (eq != std::string::npos) {
                cb::AppOptions::SetVar entry;
                entry.frame = frame_no;
                entry.id = std::atoi(spec.c_str());
                entry.value = spec.substr(eq + 1);
                opts.set_vars.push_back(entry);
            }
        } else if (a == "--profile") {
            opts.profile = next("");
        } else if (a == "--fb-url") {
            opts.fb_url = next("");
        } else if (a == "--fb-on") {
            opts.fb_auto = true;         // сеть на десктопе с адресом по умолчанию
        } else if (a == "--fb-offline") {
            opts.fb_offline = true;
        } else if (a == "--realtime") {
            opts.realtime = true;        // кадры идут по часам, а не максимально быстро
        } else if (a == "--trace") {
            opts.trace_object = std::atoi(next("-1").c_str());
            opts.trace_objects.push_back(opts.trace_object);
        } else if (a == "--verbose") {
            opts.verbose = true;
        } else if (a == "--help" || a == "-h") {
            std::printf(
                "cb-classic — порт Cubic battle (Pocket Code) на C/C++/Lua\n\n"
                "  --root DIR     корень проекта (по умолчанию .)\n"
                "  --scene ID     стартовая сцена (0 = первая)\n"
                "  --frames N     сколько кадров выполнить\n"
                "  --fps N        частота кадров (по умолчанию 30)\n"
                "  --out DIR      куда писать кадры PNG\n"
                "  --script FILE  сценарий ввода (press/release/move/back/scene)\n"
                "  --size WxH     размер кадра\n"
                "  --profile P    стартовые переменные: \"61=Гость,62=123,60=3\"\n"
                "  --set-var S    выставить переменную на кадре: \"4=0@20\"\n"
                "  --no-sound     без звука\n"
                "  --pixel-scale N рисовать кадр в N раз крупнее (адаптация экрана)\n"
                "  --bench N      замерить время кадра (без записи PNG)\n"
                "  --window       открыть окно SDL2 (нужна сборка make SDL2=1)\n"
                "  --window-frames N  оконный режим: выйти после N кадров\n"
                "  --screenshot F сохранить кадр окна в PNG\n"
                "  --check DIR    проверить синтаксис Lua-файлов в каталоге\n"
                "  --trace ID     печатать состояние объекта (id) каждый кадр\n"
                "  --fb-url URL   база Firebase для онлайна (на десктопе — http, для тестов)\n"
                "  --fb-on        включить сеть с адресом по умолчанию\n"
                "  --fb-offline   без сети совсем\n"
                "  --realtime     идти в реальном времени (нужно сети в headless-прогоне)\n"
                "  --seed N       зерно случайности (по умолчанию — из часов)\n"
                "  --verbose      подробный вывод\n");
            return 0;
        }
    }

    if (!seed_given && !opts.headless) {
        // В игре зерно случайности берём из часов: иначе в каждом бою выпадал бы
        // один и тот же скин врага (и одно и то же интро), а яйца — один и тот же
        // лут. В headless-прогонах (тесты, снимки экрана) остаётся фиксированное
        // зерно, чтобы проверки были повторяемыми.
        opts.seed = static_cast<unsigned>(std::time(nullptr));
    }

    if (!opts.check_dir.empty()) {
        // Проверка синтаксиса сгенерированных Lua-модулей (без запуска игры).
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        int checked = 0;
        int failed = 0;
        std::error_code ec;
        for (const auto &entry : std::filesystem::recursive_directory_iterator(opts.check_dir, ec)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".lua") {
                continue;
            }
            std::string path = entry.path().string();
            if (luaL_loadfile(L, path.c_str()) != LUA_OK) {
                std::fprintf(stderr, "%s: %s\n", path.c_str(), lua_tostring(L, -1));
                lua_pop(L, 1);
                failed++;
            }
            checked++;
        }
        lua_close(L);
        std::printf("проверено файлов Lua: %d, ошибок: %d\n", checked, failed);
        return failed == 0 ? 0 : 1;
    }

    cb::App app(opts);
    if (!app.init()) {
        std::fprintf(stderr, "инициализация не удалась\n");
        return 1;
    }

    if (opts.check_buttons) {
        return app.check_buttons() ? 0 : 1;
    }

#ifdef CB_USE_SDL2
    if (!opts.headless) {
        // Оконный режим: тот же движок, кадры отдаются в SDL-текстуру.
        // адаптация экрана: оригинал свёрстан в 1348x720 (19:10), а телефоны
        // бывают 20:9 и шире — поэтому окно полноэкранное, а сцена
        // вписывается через логический размер (letterbox, без обрезки и
        // растяжения). Тач-координаты пересчитываются через ту же матрицу.
        SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
        SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
        SDL_SetHint(SDL_HINT_ANDROID_BLOCK_ON_PAUSE, "1");
        SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            std::fprintf(stderr, "SDL2: %s\n", SDL_GetError());
            return 1;
        }
#ifdef __ANDROID__
        Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP;
#else
        Uint32 window_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
#endif
        SDL_Window *window = SDL_CreateWindow(
            "Cubic battle — C/C++/Lua", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            app.options.width, app.options.height, window_flags);
        if (!window) {
            std::fprintf(stderr, "SDL2: %s\n", SDL_GetError());
            SDL_Quit();
            return 1;
        }
        int win_w = app.options.width;
        int win_h = app.options.height;
        SDL_GetWindowSize(window, &win_w, &win_h);
        // Масштаб кадра: на телефоне рисуем в разрешении экрана, чтобы шрифт
        // и спрайты были чёткими. Берём меньший из двух коэффициентов — тогда
        // пропорции сцены сохраняются, а лишнее уходит в поля (letterbox).
        double pixel_scale = app.options.pixel_scale;
        if (pixel_scale <= 1.0) {
            double by_height = static_cast<double>(win_h) / static_cast<double>(app.options.height);
            double by_width = static_cast<double>(win_w) / static_cast<double>(app.options.width);
            pixel_scale = std::min(2.0, std::min(by_height, by_width));
            if (pixel_scale < 1.0) {
                pixel_scale = 1.0;
            }
        }
        if (pixel_scale > 1.01) {
            app.renderer = cb::Renderer(app.options.width, app.options.height, pixel_scale);
            app.engine.set_text_measurer([&app](const std::string &text, double size) {
                return app.renderer.text_width(text, size);
            });
            std::printf("адаптация: сцена %dx%d, буфер %dx%d (x%.2f), экран %dx%d\n",
                        app.options.width, app.options.height, app.renderer.width(),
                        app.renderer.height(), pixel_scale, win_w, win_h);
        }
        // Вертикальная синхронизация: без неё цикл крутится сотни раз в секунду,
        // ест процессор и картинка дёргается.
        SDL_Renderer *renderer = SDL_CreateRenderer(
            window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        bool vsync = renderer != nullptr;
        if (!renderer) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        }
        if (!renderer) {
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
            vsync = false;
        }
        if (!renderer) {
            std::fprintf(stderr, "SDL2: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            SDL_Quit();
            return 1;
        }
        if (vsync) {
            SDL_RendererInfo info;
            if (SDL_GetRendererInfo(renderer, &info) == 0) {
                vsync = (info.flags & SDL_RENDERER_PRESENTVSYNC) != 0;
            }
        }
        // Звук: устройство открывается после SDL_Init (музыки в оригинале нет).
        app.open_audio();
        // Кадр рисуется SDL-текстурами (GPU). Программный app.renderer остаётся
        // источником размеров/шрифта — по нему же снимаются PNG в headless.
        cb::SdlView view(renderer, app.renderer);
        SDL_RenderSetLogicalSize(renderer, view.width(), view.height());
        std::printf("вид: SDL2%s, кадр %dx%d\n", vsync ? " (vsync)" : "", view.width(),
                    view.height());

        // окно -> координаты сцены (учитывает letterbox и полноэкранный режим)
        auto to_stage = [&](double wx, double wy, double *sx, double *sy) {
            float lx = 0.0f;
            float ly = 0.0f;
            SDL_RenderWindowToLogical(renderer, static_cast<int>(wx), static_cast<int>(wy),
                                      &lx, &ly);
            *sx = (static_cast<double>(lx) - view.width() / 2.0) / view.scale();
            *sy = (static_cast<double>(view.height()) / 2.0 - static_cast<double>(ly)) /
                  view.scale();
        };
        auto finger_x = [&](float nx) { return static_cast<double>(nx) * win_w; };
        auto finger_y = [&](float ny) { return static_cast<double>(ny) * win_h; };
        bool running = true;
        int drawn_frames = 0;
        Uint32 start_ticks = SDL_GetTicks();
        Uint32 last = SDL_GetTicks();
        const Uint32 frame_ms = 1000u / 60u;
        std::string last_focus;
        auto sync_text_input = [&]() {
            std::string focus = app.engine.focused_text();
            if (focus == last_focus) {
                return;
            }
            last_focus = focus;
            if (focus.empty()) {
                SDL_StopTextInput();       // на Android скрывает экранную клавиатуру
            } else {
                SDL_StartTextInput();
            }
        };
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_FINGERDOWN) {
                    // каждый палец — отдельное касание: джойстик и кнопки
                    // работают одновременно
                    double x = 0.0;
                    double y = 0.0;
                    to_stage(finger_x(event.tfinger.x), finger_y(event.tfinger.y), &x, &y);
                    app.engine.touch_begin(static_cast<int>(event.tfinger.fingerId) + 1, x, y);
                } else if (event.type == SDL_FINGERMOTION) {
                    double x = 0.0;
                    double y = 0.0;
                    to_stage(finger_x(event.tfinger.x), finger_y(event.tfinger.y), &x, &y);
                    app.engine.touch_move(static_cast<int>(event.tfinger.fingerId) + 1, x, y);
                } else if (event.type == SDL_FINGERUP) {
                    app.engine.touch_end(static_cast<int>(event.tfinger.fingerId) + 1);
                } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    double x = 0.0;
                    double y = 0.0;
                    to_stage(event.button.x, event.button.y, &x, &y);
                    app.engine.touch_begin(0, x, y);
                } else if (event.type == SDL_MOUSEBUTTONUP) {
                    app.engine.touch_end(0);
                } else if (event.type == SDL_MOUSEMOTION) {
                    if (app.engine.touch_down()) {
                        double x = 0.0;
                        double y = 0.0;
                        to_stage(event.motion.x, event.motion.y, &x, &y);
                        app.engine.touch_move(0, x, y);
                    }
                } else if (event.type == SDL_TEXTINPUT) {
                    app.engine.text_input(event.text.text);
                } else if (event.type == SDL_KEYDOWN) {
                    switch (event.key.keysym.sym) {
                        case SDLK_BACKSPACE:
                            app.engine.text_backspace();
                            break;
                        case SDLK_RETURN:
                        case SDLK_KP_ENTER:
                            if (!app.engine.focused_text().empty()) {
                                app.engine.blur_text();
                            }
                            break;
                        case SDLK_AC_BACK:
                        case SDLK_ESCAPE:
                            // на Android системная «назад» приходит как SDLK_AC_BACK
                            if (!app.engine.focused_text().empty()) {
                                app.engine.blur_text();
                            } else {
                                app.apply_input_command("back");
                            }
                            break;
                        default:
                            break;
                    }
                }
            }
            Uint32 now = SDL_GetTicks();
            double dt = (now - last) / 1000.0;
            last = now;
            if (dt <= 0 || dt > 0.25) {
                dt = 1.0 / 60.0;
            }
            app.step(dt);
            sync_text_input();
            Uint32 frame_start = SDL_GetTicks();
            view.draw_scene(app.engine);
            SDL_RenderPresent(renderer);
            ++drawn_frames;
            if (!vsync) {
                // без vsync придерживаем кадр до 60 Гц, иначе цикл жрёт батарею
                Uint32 spent = SDL_GetTicks() - frame_start;
                if (spent < frame_ms) {
                    SDL_Delay(frame_ms - spent);
                }
            }
            if (app.options.window_frames > 0 && drawn_frames >= app.options.window_frames) {
                running = false;
            }
        }
        if (drawn_frames > 0) {
            Uint32 total_ms = SDL_GetTicks() - start_ticks;
            double fps = total_ms > 0 ? 1000.0 * drawn_frames / total_ms : 0.0;
            std::printf("кадров: %d, %.1f fps%s\n", drawn_frames, fps,
                        vsync ? " (vsync)" : "");
        }
        if (!app.options.screenshot.empty()) {
            // кадр перерисовывается перед снимком: после SDL_RenderPresent
            // содержимое буфера обмена буферами не определено
            view.draw_scene(app.engine);
            int out_w = 0;
            int out_h = 0;
            if (SDL_GetRendererOutputSize(renderer, &out_w, &out_h) == 0 && out_w > 0 &&
                out_h > 0) {
                std::vector<unsigned char> shot(static_cast<size_t>(out_w) * out_h * 4);
                if (SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_RGBA32, shot.data(),
                                         out_w * 4) == 0) {
                    cb_image image;
                    image.width = out_w;
                    image.height = out_h;
                    image.pixels = shot.data();
                    cb_png_save(app.options.screenshot.c_str(), &image);
                    std::printf("скриншот: %s (%dx%d)\n", app.options.screenshot.c_str(),
                                out_w, out_h);
                }
            }
        }
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
#endif

    app.run();
    return 0;
}
