-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 96 "Достижения" / объект 429 "Скролл"
local E = require("runtime.engine")

return {
  scene_id = 96,
  object_id = 429,
  name = "Скролл",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_x(o, E.op_neg(550))
        E.set_size(o, 7000)
        E.set_rot(o, 90)
        E.camera_off()
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        while E.truthy(E.touch_active()) do
          E.set_y(o, E.touch_y())
          E.set_gv(48, E.op_neg(E.op_div(E.pos_y(o), 25)))
          E.broadcast(2)
          E.frame()
        end
      end,
    },
    { -- скрипт 3: tap
      trigger = "tap",
      body = function(o)
        E.set_y(o, 0)
      end,
    },
  },
}
