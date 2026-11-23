-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 80 "Классы" / объект 422 "Лошадь"
local E = require("runtime.engine")

return {
  scene_id = 80,
  object_id = 422,
  name = "Лошадь",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_sub(E.op_sub(E.op_sub(E.op_sub(E.op_sub(25, 160), 160), 160), 160), 160))
        E.set_rot(o, E.op_neg(90))
        E.hide(o)
        if E.truthy(E.op_eq(E.random(1, 100), 1)) then
          E.show(o)
        end
        E.call_function(2)
        E.add_y(o, E.gv(48))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(47, 5)
      end,
    },
  },
}
