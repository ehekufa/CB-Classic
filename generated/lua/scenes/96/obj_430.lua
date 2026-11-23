-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 96 "Достижения" / объект 430 "KI KI MA MA"
local E = require("runtime.engine")

return {
  scene_id = 96,
  object_id = 430,
  name = "KI KI MA MA",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 25)
        E.set_size(o, 130)
        E.call_function(2)
        E.add_y(o, E.gv(48))
      end,
    },
  },
}
