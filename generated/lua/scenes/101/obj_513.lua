-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 101 "Конец" / объект 513 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 101,
  object_id = 513,
  name = "Фон",
  scripts = {
    { -- скрипт 1: back
      trigger = "back",
      body = function(o)
        E.exit_game()
      end,
    },
  },
}
