// main.cpp — точка входа: headless-прогон и (опционально) окно через SDL2.
//
// Примеры:
//   ./cb-classic --frames 90 --scene 1 --out out/frames
//   ./cb-classic --script tests/scripts/click_menu.txt --frames 200 --dump-state
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
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
#include <SDL2/SDL.h>
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

    if (options.width <= 0) {
        options.width = engine.stage_width();
    }
    if (options.height <= 0) {
        options.height = engine.stage_height();
    }
    renderer = Renderer(options.width, options.height);
    return ok;
}

void App::step(double dt)
{
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
        double x = 0, y = 0;
        in >> x >> y;
        engine.set_touch(engine.touch_down(), x, y, engine.touch_count());
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

bool App::run()
{
    mkdir_p(options.out_dir);
    auto script = options.script.empty() ? std::vector<std::string>()
                                         : read_lines(options.script);
    size_t script_pos = 0;
    double dt = 1.0 / options.fps;
    int frames = options.frames > 0 ? options.frames : 1;

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
        step(dt);
        char name[256];
        std::snprintf(name, sizeof(name), "%s/frame_%04d.png", options.out_dir.c_str(), frame);
        render_to_disk(name);
        if (options.verbose && frame < 8) {
            std::fprintf(stderr, "[кадр %d] сцена=%d журнал=%zu спрайтов=%zu\n", frame,
                         engine.current_scene_id(), engine.log_lines().size(),
                         engine.sprites_in_draw_order().size());
        }
        if (options.verbose && frame % 30 == 0) {
            std::printf("кадр %d: сцена %d, спрайтов %d\n", frame,
                        engine.current_scene_id(), renderer.drawn_sprites());
        }
    }

    std::printf("сцена: %d (%s), кадров: %d\n", engine.current_scene_id(),
                engine.current_scene_name().c_str(), frames);
    std::printf("спрайтов на сцене: %zu, записей журнала: %zu\n",
                engine.sprites_in_draw_order().size(), engine.log_lines().size());
    std::printf("прозвучало звуков: %zu\n", engine.sound_log().size());
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
        std::printf("\n%-6s %-22s %8s %8s %7s %7s %4s %-30s\n",
                    "handle", "имя", "x", "y", "size", "alpha", "vis", "картинка");
        for (cb::Sprite *sprite : engine.sprites_in_draw_order()) {
            const cb::ImageRef *ref = sprite->current_image();
            std::printf("%-6d %-22s %8.1f %8.1f %7.1f %7.1f %4d %s (%dx%d)\n",
                        sprite->handle, sprite->name.c_str(), sprite->x, sprite->y,
                        sprite->size, sprite->alpha, sprite->visible ? 1 : 0,
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

int main(int argc, char **argv)
{
    cb::AppOptions opts;
    std::vector<std::string> args(argv + 1, argv + argc);

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string &a = args[i];
        auto next = [&](const char *def) -> std::string {
            return (i + 1 < args.size()) ? args[++i] : def;
        };
        if (a == "--root") {
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
        } else if (a == "--check") {
            opts.check_dir = next("generated/lua");
        } else if (a == "--profile") {
            opts.profile = next("");
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
                "  --check DIR    проверить синтаксис Lua-файлов в каталоге\n"
                "  --verbose      подробный вывод\n");
            return 0;
        }
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

#ifdef CB_USE_SDL2
    if (!opts.headless) {
        // Оконный режим: тот же движок, кадры отдаются в SDL-текстуру.
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
            std::fprintf(stderr, "SDL2: %s\n", SDL_GetError());
            return 1;
        }
        SDL_Window *window = SDL_CreateWindow(
            "Cubic battle — C/C++/Lua", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            app.options.width, app.options.height, SDL_WINDOW_SHOWN);
        SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                                                 SDL_TEXTUREACCESS_STREAMING,
                                                 app.options.width, app.options.height);
        bool running = true;
        Uint32 last = SDL_GetTicks();
        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    running = false;
                } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                    double x = event.button.x - app.options.width / 2.0;
                    double y = app.options.height / 2.0 - event.button.y;
                    app.engine.push_touch_press(x, y);
                } else if (event.type == SDL_MOUSEBUTTONUP) {
                    app.engine.set_touch(false, app.engine.touch_x(), app.engine.touch_y(), 0);
                } else if (event.type == SDL_MOUSEMOTION) {
                    if (app.engine.touch_down()) {
                        double x = event.motion.x - app.options.width / 2.0;
                        double y = app.options.height / 2.0 - event.motion.y;
                        app.engine.set_touch(true, x, y, 1);
                    }
                } else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
                    app.apply_input_command("back");
                }
            }
            Uint32 now = SDL_GetTicks();
            double dt = (now - last) / 1000.0;
            last = now;
            if (dt <= 0 || dt > 0.25) {
                dt = 1.0 / 60.0;
            }
            app.step(dt);
            app.renderer.draw_scene(app.engine);
            SDL_UpdateTexture(texture, nullptr, app.renderer.pixels(), app.options.width * 4);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }
#endif

    app.run();
    return 0;
}
