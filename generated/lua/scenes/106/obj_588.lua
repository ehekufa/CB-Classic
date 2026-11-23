-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 106 "Плохая концовка" / объект 588 "Пианино"
local E = require("runtime.engine")

return {
  scene_id = 106,
  object_id = 588,
  name = "Пианино",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_neg(200))
        E.wait(E.num(16))
        E.transition(o, 1.2, E.pos_x(o), E.op_neg(600))
      end,
    },
  },
}
