-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 105 "GoldenEgg" / объект 585 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 105,
  object_id = 585,
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
