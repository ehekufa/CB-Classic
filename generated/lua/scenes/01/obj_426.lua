-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 426 "Достижения"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 426,
  name = "Достижения",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(400), E.op_sub(E.op_sub(E.op_sub(40, 95), 95), 95))
        E.hide(o)
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
        E.wait(E.num(0.06))
        E.run_scene(96) return
      end,
    },
  },
}
