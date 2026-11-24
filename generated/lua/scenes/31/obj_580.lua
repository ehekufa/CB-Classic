-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 31 "Больше" / объект 580 "Очистить прогресс"
local E = require("runtime.engine")

return {
  scene_id = 31,
  object_id = 580,
  name = "Очистить прогресс",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(440), E.op_sub(E.op_sub(E.op_sub(230, 95), 95), 95))
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
        E.ask(o, "Введите ABC для подтверждения", function(answer) E.set_lv(o, 1, answer) E.call_function(1) end)
      end,
    },
    { -- скрипт 4: function:1
      trigger = "function:1",
      body = function(o)
        if E.truthy(E.op_eq(E.lv(o, 1), "ABC")) then
          E.set_gv(80, 0)
          E.set_gv(47, 0)
          E.set_gv(24, 0)
          E.set_gv(93, 0)
          E.array_clear(1)
          E.array_clear(2)
          E.store_var(o, E.gv(80), 80)
          E.store_var(o, E.gv(47), 47)
          E.store_var(o, E.gv(24), 24)
          E.array_save(1)
          E.array_save(2)
          E.toast("Прогресс сброшен!")
          E.run_scene(1) return
        end
        if E.truthy(E.op_ne(E.lv(o, 1), "ABC")) then
          E.run_scene(1) return
        end
      end,
    },
  },
}
