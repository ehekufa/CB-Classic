-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 96 "Достижения" / объект 431 "Титан"
local E = require("runtime.engine")

return {
  scene_id = 96,
  object_id = 431,
  name = "Титан",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_sub(25, 95))
        E.set_size(o, 130)
        E.call_function(2)
        E.add_y(o, E.gv(48))
      end,
    },
  },
}
