-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 517 "выбор (1)"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 517,
  name = "выбор (1)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.camera_off()
        E.hide(o)
        while true do
          E.go_to(o, 516)
          E.add_x(o, 200)
          E.add_y(o, E.op_neg(100))
          E.frame()
        end
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(94, "(Админ)DerKa: ")
      end,
    },
    { -- скрипт 3: tap
      trigger = "tap",
      body = function(o)
        E.broadcast(21)
        E.call_function(23)
        E.hide(o)
        E.call_function(21)
        E.hide(o)
        E.call_function(22)
        E.show(o)
      end,
    },
  },
}
