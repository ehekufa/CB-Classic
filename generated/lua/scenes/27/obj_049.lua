-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 27 "Кубы" / объект 49 "Камера"
local E = require("runtime.engine")

return {
  scene_id = 27,
  object_id = 49,
  name = "Камера",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.camera_focus(o, 0, 0)
      end,
    },
    { -- скрипт 2: function:3
      trigger = "function:3",
      body = function(o)
        E.add_y(o, 80)
      end,
    },
    { -- скрипт 3: function:2
      trigger = "function:2",
      body = function(o)
        E.add_y(o, E.op_neg(80))
      end,
    },
  },
}
