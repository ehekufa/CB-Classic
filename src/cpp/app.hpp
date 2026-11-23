// app.hpp — связка «движок + Lua + рендерер».
#ifndef CB_APP_HPP
#define CB_APP_HPP

#include <string>
#include <vector>

#include "audio.hpp"
#include "engine.hpp"
#include "renderer.hpp"

struct lua_State;

namespace cb {

using Eng = Engine;

struct AppOptions {
    std::string root = ".";              // корень репозитория (там assets/)
    int scene = 0;                       // стартовая сцена (0 = первая в проекте)
    int width = 0;                       // 0 = размер сцены
    double pixel_scale = 1.0;            // суперсэмплинг (на телефоне ставится сам)
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
    bool check_buttons = false;          // проверить, что все кнопки получают нажатие
    int bench = 0;                       // замерить время кадра (без записи PNG)
    int window_frames = 0;               // оконный режим: выйти после N кадров (0 = бесконечно)
    std::string screenshot;              // сохранить последний кадр окна в PNG
    bool no_sound = false;               // без звука вообще
    int trace_object = -1;               // --trace ID: печатать состояние объекта по кадрам
    // --set-var "id=значение@кадр" — установить переменную на нужном кадре
    struct SetVar { int frame = 0; int id = 0; std::string value; };
    std::vector<SetVar> set_vars;
};

class App {
public:
    explicit App(AppOptions options);
    ~App();

    bool init();
    void step(double dt);
    void render_to_disk(const std::string &path);
    void trace_object(int object_id, int frame);
    bool run();
    void shutdown();

    // выполняет одну строку сценария ввода: press/release/back/scene
    void apply_input_command(const std::string &line);
    // Открывает звуковое устройство (нужен уже инициализированный SDL)
    bool open_audio();
    // Диагностика: для всех сцен печатает кнопки, до которых не доходит нажатие.
    bool check_buttons();

    Engine engine;
    Renderer renderer;
    Audio audio;
    AppOptions options;
    lua_State *lua = nullptr;

private:
    int frame_index_ = 0;               // номер кадра (для --set-var)
    bool open_lua();
    void close_lua();
};

void register_api(lua_State *L, App *app);

}  // namespace cb

#endif  // CB_APP_HPP
