-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 77 "Регистрация" / объект 165 "Поле ввода визуал 2"
local E = require("runtime.engine")

return {
  scene_id = 77,
  object_id = 165,
  name = "Поле ввода визуал 2",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_neg(40))
        E.text_create("Пароль", "on", 450, 80, 62)
        E.text_set("Пароль", 'pos', E.pos_x(o), E.pos_y(o))
        E.text_set("Пароль", 'placeholder', "Пароль")
        E.text_set("Пароль", 'align', "center")
      end,
    },
  },
}
