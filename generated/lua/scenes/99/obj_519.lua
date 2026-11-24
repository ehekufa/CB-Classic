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
        E.camera_off(o)
        E.hide(o)
        E.forever(function()
          E.go_to(o, 516)
          E.add_x(o, E.op_neg(200))
          E.add_y(o, 100)
        end)
      end,
    },
    { -- скрипт 2: touch_up
      trigger = "touch_up",
      body = function(o)
        E.broadcast(23)
      end,
    },
    { -- скрипт 3: function:22
      trigger = "function:22",
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 4: function:23
      trigger = "function:23",
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 5: function:21
      trigger = "function:21",
      body = function(o)
        E.hide(o)
      end,
    },
  },
}
