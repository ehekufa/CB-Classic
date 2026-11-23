-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 555 "Рука"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 555,
  name = "Рука",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "dynamic")
        E.set_sensor(o, "off")
        E.set_size(o, 73)
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
        E.wait_until(function() return E.truthy(E.op_eq(E.gv(8), 0)) end, o)
        E.set_image_by_id(o, 1081)
        E.wait(E.num(0.07))
        E.set_image_by_id(o, 1080)
      end,
    },
    { -- скрипт 4: function:8
      trigger = "function:8",
      body = function(o)
        E.set_image_by_id(o, 1079)
        E.wait(E.num(1))
        E.set_image_by_id(o, 1080)
      end,
    },
  },
}
