-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 305 "Закрыть"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 305,
  name = "Закрыть",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 65, 220)
        E.hide(o)
        E.camera_off(o)
      end,
    },
    { -- скрипт 2: function:11
      trigger = "function:11",
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 3: touch_down
      trigger = "touch_down",
      body = function(o)
        E.hide(o)
        E.broadcast_to_object(304, 12)
        E.broadcast_to_object(309, 12)
      end,
    },
  },
}
