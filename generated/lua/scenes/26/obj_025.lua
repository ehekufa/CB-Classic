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
        E.camera_off()
        E.set_pos(o, 590, E.op_neg(150))
        E.set_size(o, 52)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(8, true)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: tap
      trigger = "tap",
      body = function(o)
        E.set_gv(8, 0)
        E.set_bright(o, 100)
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_lt(E.gv(4), 1)) end,
      body = function(o)
        E.wait(E.num(0.03))
        E.add_gv(24, 5)
        E.store_var(o, E.gv(24))
        if E.truthy(E.op_ne(E.gv(60), 3)) then
          E.load_var(o, E.gv(24))
          E.fb_set(E.fb_key(E.gv(61), E.gv(62), "К"), E.gv(24), "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
        E.run_scene(1) return
      end,
    },
  },
}
