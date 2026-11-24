-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 101 "Конец" / объект 514 "Куб"
local E = require("runtime.engine")

return {
  scene_id = 101,
  object_id = 514,
  name = "Куб",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(1000), E.op_neg(75))
        E.transition(o, 3, E.op_neg(250), E.pos_y(o))
      end,
    },
  },
}
