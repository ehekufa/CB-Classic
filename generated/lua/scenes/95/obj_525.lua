-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 525 "Игрок 5 (админ) (1)"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 525,
  name = "Игрок 5 (админ) (1)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_size(o, 350)
        while true do
          E.fb_get(E.gv(102), "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(102, v) end)
          E.fb_get("X404", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(103, v) end)
          E.fb_get("Y404", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(104, v) end)
          E.set_pos(o, E.gv(103), E.gv(104))
          E.set_rot(o, E.gv(102))
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
