-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 77 "Регистрация" / объект 164 "Поле ввода визуал"
local E = require("runtime.engine")

return {
  scene_id = 77,
  object_id = 164,
  name = "Поле ввода визуал",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 50)
        E.text_create("Имя", "on", 450, 80, 61)
        E.text_set("Имя", 'pos', E.pos_x(o), E.pos_y(o))
        E.text_set("Имя", 'placeholder', "Имя")
        E.text_set("Имя", 'align', "center")
      end,
    },
  },
}
