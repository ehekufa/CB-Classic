-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 491 "Игрок 5 (админ)"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 491,
  name = "Игрок 5 (админ)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_size(o, 350)
        E.forever(function()
          E.fb_get("Rotate5", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(96, v) end)
          E.fb_get("X5", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(88, v) end)
          E.fb_get("Y5", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(89, v) end)
          E.set_pos(o, E.gv(88), E.gv(89))
          E.set_rot(o, E.gv(96))
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
          E.fb_set("X4", E.pos_x(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.fb_set("Y1", E.pos_y(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.set_pos(o, E.gv(73), E.gv(74))
          E.frame()
        end
      end,
    },
  },
}
