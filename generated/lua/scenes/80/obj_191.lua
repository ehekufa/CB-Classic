-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 80 "Классы" / объект 191 "Скролл"
local E = require("runtime.engine")

return {
  scene_id = 80,
  object_id = 191,
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
        E.set_y(o, 0)
        while E.truthy(E.touch_active()) do
          E.set_y(o, E.touch_y())
          E.set_gv(48, E.op_neg(E.op_div(E.pos_y(o), 30)))
          E.broadcast(2)
          E.frame()
        end
      end,
    },
    { -- скрипт 3: touch_up
      trigger = "touch_up",
      body = function(o)
        -- (пусто)
      end,
    },
  },
}
