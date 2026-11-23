-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 96 "Достижения" / объект 432 "Снеговик"
local E = require("runtime.engine")

return {
  scene_id = 96,
  object_id = 432,
  name = "Снеговик",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_sub(E.op_sub(25, 95), 95))
        if E.truthy(E.op_lt(E.gv(80), 35)) then
          E.set_bright(o, 60)
        end
        E.call_function(2)
        E.add_y(o, E.gv(48))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_ge(E.gv(80), 35)) then
          E.set_gv(47, 3)
          E.toast("Класс \"снеговика\" экипирован:")
        else
          E.toast("ов")
        end
      end,
    },
  },
}
