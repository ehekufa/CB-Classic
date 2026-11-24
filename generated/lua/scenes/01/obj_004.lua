-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 4 "Надпись"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 4,
  name = "Надпись",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(425), 180)
        E.set_size(o, 85)
      end,
    },
  },
}
