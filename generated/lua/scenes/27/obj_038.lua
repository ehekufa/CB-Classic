-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 27 "Кубы" / объект 38 "Назад"
local E = require("runtime.engine")

return {
  scene_id = 27,
  object_id = 38,
  name = "Назад",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 440, 175)
        E.camera_off()
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: touch_up
      trigger = "touch_up",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_bright(o, 100)
        E.wait(E.num(0.06))
        E.run_scene(31) return
      end,
    },
  },
}
