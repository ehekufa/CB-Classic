-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 106 "Плохая концовка" / объект 587 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 106,
  object_id = 587,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 1000000)
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
