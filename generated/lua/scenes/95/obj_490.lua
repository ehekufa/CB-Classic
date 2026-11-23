-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 490 "Игрок 4 (админ)"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 490,
  name = "Игрок 4 (админ)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 350)
        E.add_body(o, "static")
        while true do
          E.fb_get("Rotate4", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(101, v) end)
          E.fb_get("X4", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(86, v) end)
          E.fb_get("Y4", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(87, v) end)
          E.set_pos(o, E.gv(86), E.gv(87))
          E.set_rot(o, "Rotate4")
          E.wait(E.num(0.13))
          E.frame()
        end
        E.call_function(2)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.go_steps(o, 50)
          E.set_rot(o, E.gv(2))
          E.fb_set("X4", E.pos_x(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.fb_set("Y1", E.pos_y(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.set_pos(o, E.gv(73), E.gv(74))
          E.frame()
        end
      end,
    },
  },
}
