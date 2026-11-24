-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 516 "выбор"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 516,
  name = "выбор",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 10, 100)
        E.camera_off(o)
        E.hide(o)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        while E.truthy(E.touch_active()) do
          E.go_to_touch(o)
          E.frame()
        end
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
