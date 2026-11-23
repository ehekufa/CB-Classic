// lua_api.cpp — мост Lua -> C++: таблица `cb`, которую использует рантайм.
//
// Регистрируются только «кирпичики» (геттеры/сеттеры, ввод, сцены, звук, текст).
// Вся логика блоков живёт в src/lua/runtime/engine.lua.
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include "app.hpp"

namespace cb {

namespace {

Eng *engine_of(lua_State *L)
{
    auto *app = static_cast<App *>(lua_touserdata(L, lua_upvalueindex(1)));
    return &app->engine;
}

int arg_num(lua_State *L, int idx)
{
    return static_cast<int>(luaL_optnumber(L, idx, 0));
}

// --- базовое ---------------------------------------------------------------

int l_log(lua_State *L)
{
    engine_of(L)->log(luaL_checkstring(L, 1));
    return 0;
}

int l_time(lua_State *L)
{
    lua_pushnumber(L, engine_of(L)->time());
    return 1;
}

int l_dt(lua_State *L)
{
    lua_pushnumber(L, engine_of(L)->dt());
    return 1;
}

int l_stage(lua_State *L)
{
    Eng *e = engine_of(L);
    if (lua_gettop(L) >= 2) {                       // cb.stage(w, h) — задать размер
        e->set_stage(arg_num(L, 1), arg_num(L, 2));
        return 0;
    }
    lua_pushinteger(L, e->stage_width());           // cb.stage() — узнать размер
    lua_pushinteger(L, e->stage_height());
    return 2;
}

int l_particles_create(lua_State *L)
{
    Eng *e = engine_of(L);
    e->particles_create(arg_num(L, 1), luaL_checkstring(L, 2), arg_num(L, 3));
    return 0;
}

int l_particles_move(lua_State *L)
{
    Eng *e = engine_of(L);
    e->particles_move(arg_num(L, 1), luaL_checkstring(L, 2), arg_num(L, 3), arg_num(L, 4));
    return 0;
}

int l_particles_stop(lua_State *L)
{
    Eng *e = engine_of(L);
    e->particles_stop(arg_num(L, 1), luaL_checkstring(L, 2));
    return 0;
}

int l_unsupported(lua_State *L)
{
    engine_of(L)->unsupported_calls(luaL_checkstring(L, 1));
    return 0;
}

// --- описание проекта ------------------------------------------------------

int l_scene_define(lua_State *L)
{
    Eng *e = engine_of(L);
    int id = arg_num(L, 1);
    const char *name = luaL_checkstring(L, 2);
    e->define_scene(id, name);
    return 0;
}

int l_object_define(lua_State *L)
{
    Eng *e = engine_of(L);
    ObjectDef def;
    def.id = arg_num(L, 2);
    def.name = luaL_checkstring(L, 3);
    def.x = luaL_optnumber(L, 4, 0);
    def.y = luaL_optnumber(L, 5, 0);
    def.rotation = luaL_optnumber(L, 6, 0);
    def.size = luaL_optnumber(L, 7, 100);
    def.default_image = 0;

    if (lua_istable(L, 8)) {
        size_t n = lua_rawlen(L, 8);
        for (size_t i = 1; i <= n; ++i) {
            lua_rawgeti(L, 8, static_cast<lua_Integer>(i));
            ImageRef ref;
            lua_getfield(L, -1, "id");
            ref.id = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            lua_getfield(L, -1, "path");
            ref.path = lua_tostring(L, -1) ? lua_tostring(L, -1) : "";
            lua_pop(L, 1);
            def.images.push_back(ref);
            lua_pop(L, 1);
        }
    }
    if (lua_istable(L, 9)) {
        size_t n = lua_rawlen(L, 9);
        for (size_t i = 1; i <= n; ++i) {
            lua_rawgeti(L, 9, static_cast<lua_Integer>(i));
            SoundRef ref;
            lua_getfield(L, -1, "id");
            ref.id = static_cast<int>(lua_tointeger(L, -1));
            lua_pop(L, 1);
            lua_getfield(L, -1, "path");
            ref.path = lua_tostring(L, -1) ? lua_tostring(L, -1) : "";
            lua_pop(L, 1);
            def.sounds.push_back(ref);
            lua_pop(L, 1);
        }
    }
    e->define_object(arg_num(L, 1), def);
    return 0;
}

int l_finalize(lua_State *L)
{
    engine_of(L)->finalize();
    return 0;
}

// --- сцены -----------------------------------------------------------------

int l_scene_run(lua_State *L)
{
    lua_pushboolean(L, engine_of(L)->run_scene(arg_num(L, 1)) ? 1 : 0);
    return 1;
}

int l_scene_name(lua_State *L)
{
    lua_pushstring(L, engine_of(L)->current_scene_name().c_str());
    return 1;
}

int l_scene_current(lua_State *L)
{
    lua_pushinteger(L, engine_of(L)->current_scene_id());
    return 1;
}

int l_scene_count(lua_State *L)
{
    lua_pushinteger(L, static_cast<lua_Integer>(engine_of(L)->scenes().size()));
    return 1;
}

int l_scene_at(lua_State *L)
{
    Eng *e = engine_of(L);
    int index = arg_num(L, 1);
    const auto &scenes = e->scenes();
    if (index < 1 || index > static_cast<int>(scenes.size())) {
        lua_pushinteger(L, 0);
        lua_pushstring(L, "");
        return 2;
    }
    const auto &scene = scenes[static_cast<size_t>(index - 1)];
    lua_pushinteger(L, scene.id);
    lua_pushstring(L, scene.name.c_str());
    return 2;
}

// --- объекты ---------------------------------------------------------------

int l_obj_by_id(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find_by_object(arg_num(L, 1));
    lua_pushinteger(L, sprite ? sprite->handle : 0);
    return 1;
}

int l_handles(lua_State *L)
{
    Eng *e = engine_of(L);
    auto sprites = e->sprites_in_draw_order();
    lua_newtable(L);
    int index = 1;
    for (Sprite *sprite : sprites) {
        lua_pushinteger(L, sprite->handle);
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

int l_get(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find(arg_num(L, 1));
    const char *field = luaL_checkstring(L, 2);
    if (!sprite) {
        lua_pushnil(L);
        return 1;
    }
    if (std::strcmp(field, "x") == 0) {
        lua_pushnumber(L, sprite->x);
    } else if (std::strcmp(field, "y") == 0) {
        lua_pushnumber(L, sprite->y);
    } else if (std::strcmp(field, "rotation") == 0) {
        lua_pushnumber(L, sprite->rotation);
    } else if (std::strcmp(field, "size") == 0) {
        lua_pushnumber(L, sprite->size);
    } else if (std::strcmp(field, "alpha") == 0) {
        lua_pushnumber(L, sprite->alpha);
    } else if (std::strcmp(field, "brightness") == 0) {
        lua_pushnumber(L, sprite->brightness);
    } else if (std::strcmp(field, "visible") == 0) {
        lua_pushinteger(L, sprite->visible ? 1 : 0);
    } else if (std::strcmp(field, "layer") == 0) {
        lua_pushinteger(L, sprite->layer);
    } else if (std::strcmp(field, "image_index") == 0) {
        lua_pushinteger(L, sprite->image_index + 1);
    } else if (std::strcmp(field, "image_count") == 0) {
        lua_pushinteger(L, sprite->def ? static_cast<lua_Integer>(sprite->def->images.size()) : 0);
    } else if (std::strcmp(field, "image_name") == 0) {
        const ImageRef *ref = sprite->current_image();
        lua_pushstring(L, ref ? ref->path.c_str() : "");
    } else if (std::strcmp(field, "sound_count") == 0) {
        lua_pushinteger(L, sprite->def ? static_cast<lua_Integer>(sprite->def->sounds.size()) : 0);
    } else if (std::strcmp(field, "name") == 0) {
        lua_pushstring(L, sprite->name.c_str());
    } else if (std::strcmp(field, "object_id") == 0) {
        lua_pushinteger(L, sprite->object_id);
    } else if (std::strcmp(field, "handle") == 0) {
        lua_pushinteger(L, sprite->handle);
    } else if (std::strcmp(field, "is_clone") == 0) {
        lua_pushinteger(L, sprite->is_clone ? 1 : 0);
    } else if (std::strcmp(field, "width") == 0) {
        lua_pushnumber(L, sprite_pixel_width(*sprite));
    } else if (std::strcmp(field, "height") == 0) {
        lua_pushnumber(L, sprite_pixel_height(*sprite));
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int l_set(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find(arg_num(L, 1));
    const char *field = luaL_checkstring(L, 2);
    if (!sprite) {
        return 0;
    }
    double value = luaL_optnumber(L, 3, 0);
    if (std::strcmp(field, "x") == 0) {
        sprite->x = value;
    } else if (std::strcmp(field, "y") == 0) {
        sprite->y = value;
    } else if (std::strcmp(field, "rotation") == 0) {
        sprite->rotation = std::fmod(value, 360.0);
    } else if (std::strcmp(field, "size") == 0) {
        sprite->size = value;
    } else if (std::strcmp(field, "alpha") == 0) {
        sprite->alpha = value;
    } else if (std::strcmp(field, "brightness") == 0) {
        sprite->brightness = value;
    } else if (std::strcmp(field, "visible") == 0) {
        sprite->visible = value != 0;
    } else if (std::strcmp(field, "layer") == 0) {
        sprite->layer = static_cast<int>(value);
    } else if (std::strcmp(field, "image_index") == 0) {
        if (sprite->def) {
            int count = static_cast<int>(sprite->def->images.size());
            int index = static_cast<int>(value) - 1;
            if (count > 0) {
                sprite->image_index = ((index % count) + count) % count;
            }
        }
    }
    return 0;
}

int l_set_image_id(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find(arg_num(L, 1));
    int image_id = arg_num(L, 2);
    bool found = sprite && e->set_image_by_id(*sprite, image_id);
    lua_pushinteger(L, found ? 1 : 0);
    return 1;
}

int l_next_image(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find(arg_num(L, 1));
    if (sprite && sprite->def && !sprite->def->images.empty()) {
        sprite->image_index =
            (sprite->image_index + 1) % static_cast<int>(sprite->def->images.size());
    }
    return 0;
}

int l_go_steps(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find(arg_num(L, 1));
    double steps = luaL_optnumber(L, 2, 0);
    if (sprite) {
        e->go_steps(*sprite, steps);
    }
    return 0;
}

int l_clone(lua_State *L)
{
    Eng *e = engine_of(L);
    lua_pushinteger(L, e->clone(arg_num(L, 1)));
    return 1;
}

int l_delete(lua_State *L)
{
    Eng *e = engine_of(L);
    lua_pushboolean(L, e->remove(arg_num(L, 1)) ? 1 : 0);
    return 1;
}

int l_hit(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *sprite = e->find(arg_num(L, 1));
    double x = luaL_optnumber(L, 2, 0);
    double y = luaL_optnumber(L, 3, 0);
    lua_pushinteger(L, (sprite && sprite->visible && sprite_hit(*sprite, x, y)) ? 1 : 0);
    return 1;
}

int l_hit_o(lua_State *L)
{
    Eng *e = engine_of(L);
    Sprite *a = e->find(arg_num(L, 1));
    Sprite *b = e->find(arg_num(L, 2));
    bool touching = false;
    if (a && b && a->visible && b->visible) {
        const ImageRef *ra = a->current_image();
        const ImageRef *rb = b->current_image();
        double wa = sprite_pixel_width(*a);
        double ha = sprite_pixel_height(*a);
        double wb = sprite_pixel_width(*b);
        double hb = sprite_pixel_height(*b);
        touching = ra && rb && std::fabs(a->x - b->x) < (wa + wb) / 2.0 &&
                   std::fabs(a->y - b->y) < (ha + hb) / 2.0;
    }
    lua_pushinteger(L, touching ? 1 : 0);
    return 1;
}

// --- ввод ------------------------------------------------------------------

int l_touch_down(lua_State *L)
{
    lua_pushinteger(L, engine_of(L)->touch_down() ? 1 : 0);
    return 1;
}

int l_touch_x(lua_State *L)
{
    lua_pushnumber(L, engine_of(L)->touch_x());
    return 1;
}

int l_touch_y(lua_State *L)
{
    lua_pushnumber(L, engine_of(L)->touch_y());
    return 1;
}

int l_touch_count(lua_State *L)
{
    lua_pushinteger(L, engine_of(L)->touch_count());
    return 1;
}

int l_touch_id(lua_State *L)
{
    lua_pushinteger(L, engine_of(L)->touch_down() ? 1 : 0);
    return 1;
}

// Блоки Pocket Code: touchXByIndex/touchYByIndex возвращают координату пальца,
// touchIDByIndex — его идентификатор (0, если такого пальца нет).
int l_touch_x_by_index(lua_State *L)
{
    double x = 0.0;
    engine_of(L)->touch_at(arg_num(L, 1), &x, nullptr);
    lua_pushnumber(L, x);
    return 1;
}

int l_touch_y_by_index(lua_State *L)
{
    double y = 0.0;
    engine_of(L)->touch_at(arg_num(L, 1), nullptr, &y);
    lua_pushnumber(L, y);
    return 1;
}

int l_touch_id_by_index(lua_State *L)
{
    double x = 0.0;
    double y = 0.0;
    bool exists = engine_of(L)->touch_at(arg_num(L, 1), &x, &y);
    lua_pushinteger(L, exists ? 1 : 0);
    return 1;
}

int l_poll_press(lua_State *L)
{
    double x = 0.0;
    double y = 0.0;
    if (!engine_of(L)->pop_press(&x, &y)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    return 2;
}

// Спрайты под точкой, сверху вниз — чтобы нажатие доходило до кнопки,
// даже если её прикрывает декоративный слой.
int l_sprites_at(lua_State *L)
{
    Eng *e = engine_of(L);
    double x = luaL_optnumber(L, 1, 0);
    double y = luaL_optnumber(L, 2, 0);
    auto hits = e->sprites_at(x, y);
    lua_createtable(L, static_cast<int>(hits.size()), 0);
    for (size_t i = 0; i < hits.size(); ++i) {
        lua_pushinteger(L, hits[i]->handle);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    return 1;
}

// --- текстовый ввод --------------------------------------------------------

int l_text_focus_at(lua_State *L)
{
    Eng *e = engine_of(L);
    double x = luaL_optnumber(L, 1, 0);
    double y = luaL_optnumber(L, 2, 0);
    TextField *field = e->focus_text_at(x, y);
    lua_pushinteger(L, field ? 1 : 0);
    return 1;
}

int l_text_focus(lua_State *L)
{
    engine_of(L)->focus_text(luaL_checkstring(L, 1));
    return 0;
}

int l_text_blur(lua_State *L)
{
    engine_of(L)->blur_text();
    return 0;
}

int l_text_focused(lua_State *L)
{
    std::string label = engine_of(L)->focused_text();
    if (label.empty()) {
        lua_pushnil(L);
    } else {
        lua_pushstring(L, label.c_str());
    }
    return 1;
}

int l_poll_text_commit(lua_State *L)
{
    std::string label;
    std::string value;
    if (!engine_of(L)->pop_text_commit(&label, &value)) {
        lua_pushnil(L);
        return 1;
    }
    lua_pushstring(L, label.c_str());
    lua_pushstring(L, value.c_str());
    return 2;
}

int l_text_input(lua_State *L)
{
    lua_pushinteger(L, engine_of(L)->text_input(luaL_optstring(L, 1, "")) ? 1 : 0);
    return 1;
}

int l_text_backspace(lua_State *L)
{
    lua_pushinteger(L, engine_of(L)->text_backspace() ? 1 : 0);
    return 1;
}

int l_press(lua_State *L)
{
    Eng *e = engine_of(L);
    e->push_touch_press(luaL_optnumber(L, 1, 0), luaL_optnumber(L, 2, 0));
    return 0;
}

int l_release(lua_State *L)
{
    Eng *e = engine_of(L);
    e->set_touch(false, e->touch_x(), e->touch_y(), 0);
    return 0;
}

// --- звук, текст, система --------------------------------------------------

int l_play_sound(lua_State *L)
{
    Eng *e = engine_of(L);
    e->play_sound(arg_num(L, 1), arg_num(L, 2));
    return 0;
}

int l_stop_sounds(lua_State *L)
{
    engine_of(L)->stop_sounds();
    return 0;
}

int l_toast(lua_State *L)
{
    engine_of(L)->toast(luaL_checkstring(L, 1));
    return 0;
}

int l_bg_color(lua_State *L)
{
    engine_of(L)->set_background(luaL_checkstring(L, 1));
    return 0;
}

int l_exit(lua_State *L)
{
    engine_of(L)->request_exit();
    return 0;
}

int l_text_create(lua_State *L)
{
    Eng *e = engine_of(L);
    TextField &field = e->text_ensure(luaL_checkstring(L, 1));
    field.x = luaL_optnumber(L, 2, 0);
    field.y = luaL_optnumber(L, 3, 0);
    field.width = luaL_optnumber(L, 4, 0);
    field.height = luaL_optnumber(L, 5, 0);
    field.color = luaL_optstring(L, 6, "#FFFFFF");
    field.align = luaL_optstring(L, 7, "left");
    field.editable = arg_num(L, 8) != 0;
    field.visible = true;
    return 0;
}

int l_text_set(lua_State *L)
{
    Eng *e = engine_of(L);
    const char *label = luaL_checkstring(L, 1);
    const char *field = luaL_checkstring(L, 2);
    TextField *target = e->text(label);
    if (!target) {
        return 0;
    }
    if (std::strcmp(field, "value") == 0) {
        target->value = luaL_optstring(L, 3, "");
    } else if (std::strcmp(field, "placeholder") == 0) {
        target->placeholder = luaL_optstring(L, 3, "");
    } else if (std::strcmp(field, "align") == 0) {
        target->align = luaL_optstring(L, 3, "left");
    } else if (std::strcmp(field, "color") == 0) {
        target->color = luaL_optstring(L, 3, "#FFFFFF");
    } else if (std::strcmp(field, "size") == 0) {
        target->size = luaL_optnumber(L, 3, 12);
    } else if (std::strcmp(field, "pos") == 0) {
        target->x = luaL_optnumber(L, 3, 0);
        target->y = luaL_optnumber(L, 4, 0);
    } else if (std::strcmp(field, "visible") == 0) {
        target->visible = luaL_optnumber(L, 3, 1) != 0;
    }
    return 0;
}

int l_text_delete(lua_State *L)
{
    engine_of(L)->text_remove(luaL_checkstring(L, 1));
    return 0;
}

int l_firebase_set(lua_State *L)
{
    engine_of(L)->fb_set(luaL_checkstring(L, 1), lua_tostring(L, 2) ? lua_tostring(L, 2) : "");
    return 0;
}

int l_firebase_get(lua_State *L)
{
    std::string value;
    if (engine_of(L)->fb_get(luaL_checkstring(L, 1), &value)) {
        lua_pushlstring(L, value.data(), value.size());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int l_store_set(lua_State *L)
{
    engine_of(L)->store_set(luaL_checkstring(L, 1), lua_tostring(L, 2) ? lua_tostring(L, 2) : "");
    return 0;
}

int l_store_get(lua_State *L)
{
    std::string value;
    if (engine_of(L)->store_get(luaL_checkstring(L, 1), &value)) {
        lua_pushlstring(L, value.data(), value.size());
    } else {
        lua_pushnil(L);
    }
    return 1;
}

const luaL_Reg kApi[] = {
    {"log", l_log},
    {"time", l_time},
    {"dt", l_dt},
    {"stage", l_stage},
    {"unsupported", l_unsupported},
    {"particles_create", l_particles_create},
    {"particles_move", l_particles_move},
    {"particles_stop", l_particles_stop},

    {"scene_define", l_scene_define},
    {"object_define", l_object_define},
    {"finalize", l_finalize},
    {"scene_run", l_scene_run},
    {"scene_name", l_scene_name},
    {"scene_current", l_scene_current},
    {"scene_count", l_scene_count},
    {"scene_at", l_scene_at},

    {"obj_by_id", l_obj_by_id},
    {"handles", l_handles},
    {"get", l_get},
    {"set", l_set},
    {"set_image_id", l_set_image_id},
    {"next_image", l_next_image},
    {"go_steps", l_go_steps},
    {"clone", l_clone},
    {"delete", l_delete},
    {"hit", l_hit},
    {"hit_o", l_hit_o},

    {"touch_down", l_touch_down},
    {"touch_x", l_touch_x},
    {"touch_y", l_touch_y},
    {"touch_count", l_touch_count},
    {"touch_id", l_touch_id},
    {"touch_x_by_index", l_touch_x_by_index},
    {"touch_y_by_index", l_touch_y_by_index},
    {"touch_id_by_index", l_touch_id_by_index},
    {"touch_x_by_id", l_touch_x_by_index},
    {"touch_y_by_id", l_touch_y_by_index},
    {"touch_id_by_id", l_touch_id_by_index},
    {"poll_press", l_poll_press},
    {"sprites_at", l_sprites_at},
    {"text_focus_at", l_text_focus_at},
    {"text_focus", l_text_focus},
    {"text_blur", l_text_blur},
    {"text_focused", l_text_focused},
    {"poll_text_commit", l_poll_text_commit},
    {"text_input", l_text_input},
    {"text_backspace", l_text_backspace},
    {"press", l_press},
    {"release", l_release},

    {"play_sound", l_play_sound},
    {"stop_sounds", l_stop_sounds},
    {"toast", l_toast},
    {"bg_color", l_bg_color},
    {"exit", l_exit},

    {"text_create", l_text_create},
    {"text_set", l_text_set},
    {"text_delete", l_text_delete},
    {"firebase_set", l_firebase_set},
    {"firebase_get", l_firebase_get},
    {"store_set", l_store_set},
    {"store_get", l_store_get},
    {nullptr, nullptr},
};

}  // namespace

void register_api(lua_State *L, App *app)
{
    lua_newtable(L);
    for (const luaL_Reg *reg = kApi; reg->name; ++reg) {
        lua_pushlightuserdata(L, app);
        lua_pushcclosure(L, reg->func, 1);
        lua_setfield(L, -2, reg->name);
    }
    lua_setglobal(L, "cb");
}

}  // namespace cb
