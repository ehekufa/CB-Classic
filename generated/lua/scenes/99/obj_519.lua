-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 519 "выбор (1) (1) (1)"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 519,
  name = "выбор (1) (1) (1)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.camera_off()
        E.hide(o)
        while true do
          E.go_to(o, 516)
          E.add_x(o, E.op_neg(200))
          E.add_y(o, 100)
          E.frame()
        end
      end,
    },
    { -- скрипт 2: tap
      trigger = "tap",
      body = function(o)
        E.broadcast(23)
        E.call_function(22)
        E.show(o)
        E.call_function(23)
        E.hide(o)
        E.call_function(21)
        E.hide(o)
      end,
    },
  },
}
