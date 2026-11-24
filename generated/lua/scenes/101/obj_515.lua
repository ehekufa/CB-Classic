-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 101 "Конец" / объект 515 "буК"
local E = require("runtime.engine")

return {
  scene_id = 101,
  object_id = 515,
  name = "буК",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 80, E.op_neg(75))
      end,
    },
  },
}
