-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 31 "Больше" / объект 579 "Сохранить прогресс"
local E = require("runtime.engine")

return {
  scene_id = 31,
  object_id = 579,
  name = "Сохранить прогресс",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(440), E.op_sub(E.op_sub(230, 95), 95))
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
        E.store_var(o, E.gv(80), 80)
        E.store_var(o, E.gv(47), 47)
        E.store_var(o, E.gv(24), 24)
        E.store_var(o, E.gv(12), 12)
        E.store_var(o, E.gv(93), 93)
        E.array_save(1)
        E.array_save(2)
        E.toast("Прогресс созранён!")
      end,
    },
  },
}
