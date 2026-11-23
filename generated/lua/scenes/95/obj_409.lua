-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 409 "Игрок 2"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 409,
  name = "Игрок 2",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_size(o, 350)
        E.forever(function()
          E.fb_get("X2", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(76, v) end)
          E.fb_get("Y2", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(75, v) end)
          E.set_pos(o, E.gv(76), E.gv(75))
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
          E.fb_set("X2", E.pos_x(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.fb_set("Y2", E.pos_y(o), "https://cubic-battle-default-rtdb.firebaseio.com/")
          E.set_pos(o, E.gv(76), E.gv(75))
          E.frame()
        end
      end,
    },
    { -- скрипт 3: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.fb_get("Какой скин у второго куба", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(106, v) end)
          E.set_image_by_id(o, E.gv(106))
          E.wait(E.num(0.05))
        end)
      end,
    },
  },
}
