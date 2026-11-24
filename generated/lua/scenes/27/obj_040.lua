-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 27 "Кубы" / объект 40 "Куб-мясник"
local E = require("runtime.engine")

return {
  scene_id = 27,
  object_id = 40,
  name = "Куб-мясник",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_add(E.op_neg(500), 340), 130)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_size(o, 95)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: touch_up
      trigger = "touch_up",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_size(o, 100)
        E.set_bright(o, 100)
        E.set_gv(12, 2)
        E.wait(E.num(0.05))
        E.run_scene(1) return
      end,
    },
  },
}
