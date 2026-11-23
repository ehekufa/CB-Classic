-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 511 "Птицетошка"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 511,
  name = "Птицетошка",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.hide(o)
        E.forever(function()
          E.wait(E.num(0.1))
          if E.truthy(E.op_eq(E.gv(82), "1")) then
            E.show(o)
          else
            E.hide(o)
          end
        end)
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.transition(o, E.random(1, 1.5), E.random(E.op_neg(500), 500), E.random(E.op_neg(100), 100))
        end)
      end,
    },
    { -- скрипт 3: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.set_rot(o, E.op_sub(E.rot(o), 0.4))
        end)
      end,
    },
  },
}
