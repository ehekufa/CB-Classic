-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 498 "Admin panel"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 498,
  name = "Admin panel",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 212.49996948242, 41.184587082513)
        E.set_size(o, 60)
        E.camera_off(o)
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
    { -- скрипт 3: function:17
      trigger = "function:17",
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 4: function:18
      trigger = "function:18",
      body = function(o)
        E.show(o)
      end,
    },
  },
}
