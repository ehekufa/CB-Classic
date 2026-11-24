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
        E.camera_off(o)
        E.set_size(o, 30)
        E.forever(function()
          E.go_to(o, 498)
          E.add_y(o, E.op_neg(50))
        end)
      end,
    },
    { -- скрипт 2: function:17
      trigger = "function:17",
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 3: function:18
      trigger = "function:18",
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 4: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 130)
        E.ask(o, "какой админ (1/3)", function(answer) E.set_gv(90, answer) end)
      end,
    },
    { -- скрипт 5: touch_up
      trigger = "touch_up",
      body = function(o)
        E.set_bright(o, 120)
      end,
    },
  },
}
