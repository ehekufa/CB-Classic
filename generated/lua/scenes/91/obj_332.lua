-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 91 "Дед мороз лаки блок" / объект 332 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 91,
  object_id = 332,
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
        E.exit_game()
      end,
    },
  },
}
