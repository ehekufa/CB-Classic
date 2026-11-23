-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 31 "Больше" / объект 33 "Кубы"
local E = require("runtime.engine")

return {
  scene_id = 31,
  object_id = 33,
  name = "Кубы",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(440), E.op_sub(230, 95))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: touch_up
      trigger = "touch_up",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_bright(o, 100)
        E.broadcast_to_object(424, 2)
        E.wait(E.num(0.06))
        E.run_scene(27) return
      end,
    },
  },
}
