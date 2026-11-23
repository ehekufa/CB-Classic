-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 89 "Магазин ивента" / объект 287 "Ивент-куб"
local E = require("runtime.engine")

return {
  scene_id = 89,
  object_id = 287,
  name = "Ивент-куб",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(500), 130)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_size(o, 95)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: tap
      trigger = "tap",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_size(o, 100)
        E.set_bright(o, 100)
        if E.truthy(E.op_ge(E.gv(92), 500)) then
          E.set_gv(12, 10)
        else
          E.toast("Он стоит 500 валентинок!")
        end
        E.wait(E.num(0.05))
        E.run_scene(1) return
      end,
    },
  },
}
