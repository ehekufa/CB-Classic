-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 104 "Яйцо" / объект 557 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 104,
  object_id = 557,
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
        E.run_scene(31) return
      end,
    },
  },
}
