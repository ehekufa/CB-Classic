-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 89 "Магазин ивента" / объект 285 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 89,
  object_id = 285,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.adaptive_off()
        E.set_size(o, 150)
      end,
    },
    { -- скрипт 2: back
      trigger = "back",
      body = function(o)
        E.run_scene(1) return
      end,
    },
  },
}
