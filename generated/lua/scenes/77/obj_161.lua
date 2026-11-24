-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 77 "Регистрация" / объект 161 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 77,
  object_id = 161,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.adaptive_off()
      end,
    },
  },
}
