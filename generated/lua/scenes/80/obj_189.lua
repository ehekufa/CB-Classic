-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 80 "Классы" / объект 189 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 80,
  object_id = 189,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.adaptive_off()
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
