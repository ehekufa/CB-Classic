-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 524 "Игрок 404 (хакер)"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 524,
  name = "Игрок 404 (хакер)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_size(o, 350)
        E.forever(function()
          E.fb_get("Rotate404", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(102, v) end)
          E.fb_get("X404", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(103, v) end)
          E.fb_get("Y404", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(104, v) end)
          E.set_rot(o, E.gv(102))
          E.set_pos(o, E.gv(103), E.gv(104))
          E.wait(E.num(0.13))
        end)
      end,
    },
    { -- скрипт 2: function:2
      trigger = "function:2",
      body = function(o)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.go_steps(o, 50)
          E.set_rot(o, E.gv(2))
          E.fb_set("Rotate404", E.rot(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.fb_set("X404", E.pos_x(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.fb_set("Y404", E.pos_y(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.set_pos(o, E.gv(103), E.gv(104))
          E.frame()
        end
      end,
    },
  },
}
