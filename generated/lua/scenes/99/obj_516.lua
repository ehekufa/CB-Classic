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
        E.camera_off()
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
