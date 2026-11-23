-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 5 "Режимы"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 5,
  name = "Режимы",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(400), 40)
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
        E.run_scene(70) return
      end,
    },
  },
}
