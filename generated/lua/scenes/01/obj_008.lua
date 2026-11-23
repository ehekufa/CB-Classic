-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 8 "Путь к богатству"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 8,
  name = "Путь к богатству",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 495, 180)
        E.text_show_var(24, 365, 120, 180, "#FFFFFF", "left")
      end,
    },
  },
}
