// app.hpp — связка «движок + Lua + рендерер».
#ifndef CB_APP_HPP
#define CB_APP_HPP

#include <string>

#include "engine.hpp"
#include "renderer.hpp"

struct lua_State;

namespace cb {

using Eng = Engine;

struct AppOptions {
    std::string root = ".";              // корень репозитория (там assets/)
    int scene = 0;                       // стартовая сцена (0 = первая в проекте)
    int width = 0;                       // 0 = размер сцены
    int height = 0;
    unsigned seed = 20260802u;
    bool headless = true;
    int frames = 0;                      // сколько кадров отрисовать (headless)
    double fps = 30.0;
    std::string out_dir = "out";         // куда писать кадры
    std::string script;                  // файл сценария ввода
    std::string profile;                 // "id=значение,..." — стартовые значения переменных
    bool verbose = false;
    std::string check_dir;               // проверить синтаксис всех .lua в каталоге и выйти
};

class App {
public:
    explicit App(AppOptions options);
    ~App();

    bool init();
    void step(double dt);
    void render_to_disk(const std::string &path);
    bool run();
    void shutdown();

    // выполняет одну строку сценария ввода: press/release/back/scene
    void apply_input_command(const std::string &line);

    Engine engine;
    Renderer renderer;
    AppOptions options;
    lua_State *lua = nullptr;

private:
    bool open_lua();
    void close_lua();
};

void register_api(lua_State *L, App *app);

}  // namespace cb

#endif  // CB_APP_HPP
