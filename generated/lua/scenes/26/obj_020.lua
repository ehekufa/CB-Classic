-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 20 "Прицел"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 20,
  name = "Прицел",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_alpha(o, 10)
        E.add_body(o, "dynamic")
        E.set_sensor(o, "off")
        E.hide(o)
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.go_to(o, 21)
          E.set_rot(o, E.gv(2))
          E.go_steps(o, 25)
        end)
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(8), true)) end,
      body = function(o)
        E.show(o)
        E.wait_until(function() return E.truthy(E.op_eq(E.gv(8), 0)) end, o)
        if E.truthy(E.touching_object(o, 22)) then
          E.add_gv(4, E.op_neg(1))
          E.clone(19)
        end
        E.hide(o)
      end,
    },
  },
}
