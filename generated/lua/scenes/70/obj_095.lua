-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 70 "Режимы" / объект 95 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 70,
  object_id = 95,
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
