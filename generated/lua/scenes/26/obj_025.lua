-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 25 "Атака"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 25,
  name = "Атака",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.camera_off(o)
        E.set_pos(o, 590, E.op_neg(150))
        E.set_size(o, 52)
      end,
    },
    { -- скрипт 2: function:16
      trigger = "function:16",
      body = function(o)
        E.hide(o)
        E.wait(E.num(3.05))
        E.show(o)
      end,
    },
    { -- скрипт 3: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(8, true)
        E.set_bright(o, 85)
        E.wait(E.num(0.08))
        E.set_gv(8, 0)
        E.set_bright(o, 100)
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_le(E.gv(4), 0)) end,
      body = function(o)
        for _ = 1, math.floor(E.num(5)) do
          E.set_gv(4, 0)
          E.frame()
        end
        E.wait(E.num(0.03))
        E.add_gv(24, 10)
        if E.truthy(E.op_eq(E.gv(47), 6)) then
          E.add_gv(80, 4)
        else
          E.add_gv(80, 2)
        end
        E.store_var(o, E.gv(24), 24)
        E.store_var(o, E.gv(80), 80)
        E.run_scene(1) return
      end,
    },
    { -- скрипт 5: function:8
      trigger = "function:8",
      body = function(o)
        E.set_bright(o, 85)
        E.wait(E.num(0.9))
        E.set_bright(o, 100)
      end,
    },
  },
}
