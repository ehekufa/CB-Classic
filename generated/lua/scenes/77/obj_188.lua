-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 77 "Регистрация" / объект 188 "Не хочу"
local E = require("runtime.engine")

return {
  scene_id = 77,
  object_id = 188,
  name = "Не хочу",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 215)
        E.set_size(o, 75)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.text_delete("Имя")
        E.text_delete("Пароль")
        E.set_gv(60, 3)
        E.run_scene(1) return
      end,
    },
  },
}
