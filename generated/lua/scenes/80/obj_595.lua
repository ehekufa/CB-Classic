-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 80 "Классы" / объект 595 "Изум"
local E = require("runtime.engine")

return {
  scene_id = 80,
  object_id = 595,
  name = "Изум",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 25)
        E.set_size(o, 79)
        E.call_function(2)
        E.add_y(o, E.gv(48))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(47, 7)
        E.toast("Класс Изума экипирован:")
      end,
    },
  },
}
