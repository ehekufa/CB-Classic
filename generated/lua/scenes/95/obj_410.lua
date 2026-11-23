-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 410 "Игрок 3"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 410,
  name = "Игрок 3",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_size(o, 350)
        while true do
          E.fb_get("X3", "https://cubic-battle-base-default-rtdb.firebaseio.com/", function(v) E.set_gv(77, v) end)
          E.fb_get("Y3", "https://cubic-battle-base-default-rtdb.firebaseio.com/", function(v) E.set_gv(78, v) end)
          E.set_pos(o, E.gv(77), E.gv(78))
          E.wait(E.num(0.13))
          E.frame()
        end
        E.call_function(2)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.go_steps(o, 50)
          E.set_rot(o, E.gv(2))
          E.fb_set("X3", E.pos_x(o), "https://cubic-battle-base-default-rtdb.firebaseio.com/")
          E.fb_set("Y3", E.pos_y(o), "https://cubic-battle-base-default-rtdb.firebaseio.com/")
          E.set_pos(o, E.gv(77), E.gv(78))
          E.frame()
        end
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        while true do
          E.fb_get("какой скин у третьего куба", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(107, v) end)
          E.set_image_by_id(o, E.gv(107))
          E.wait(E.num(0.05))
          E.frame()
        end
      end,
    },
  },
}
