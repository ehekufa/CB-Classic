-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 526 "выйти из админа и зайти в другого"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 526,
  name = "выйти из админа и зайти в другого",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_bright(o, 120)
        E.camera_off()
        E.set_size(o, 30)
        while true do
          E.go_to(o, 498)
          E.add_y(o, E.op_neg(50))
          E.frame()
        end
        E.call_function(17)
        E.hide(o)
        E.call_function(18)
        E.show(o)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 130)
        E.ask(o, "какой админ (1/3)", function(answer) E.set_gv(90, answer) end)
      end,
    },
    { -- скрипт 3: tap
      trigger = "tap",
      body = function(o)
        E.set_bright(o, 120)
      end,
    },
  },
}
