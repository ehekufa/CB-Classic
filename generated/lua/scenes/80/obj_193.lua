-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 80 "Классы" / объект 193 "Азум"
local E = require("runtime.engine")

return {
  scene_id = 80,
  object_id = 193,
  name = "Азум",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 25)
      end,
    },
    { -- скрипт 2: function:2
      trigger = "function:2",
      body = function(o)
        E.add_y(o, E.gv(48))
      end,
    },
    { -- скрипт 3: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(47, 1)
        E.toast("Класс Азума экипирован:")
      end,
    },
  },
}
