-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 421 "Леденец"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 421,
  name = "Леденец",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.load_var(o, 93)
        E.set_pos(o, E.random(E.op_neg(600), 600), E.random(E.op_neg(300), 300))
        E.set_size(o, 70)
        E.add_body(o, "static")
        E.set_sensor(o, "off")
      end,
    },
    { -- скрипт 2: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 22)) end,
      body = function(o)
        E.set_pos(o, E.random(E.op_neg(600), 600), E.random(E.op_neg(300), 300))
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 21)) end,
      body = function(o)
        E.wait(E.num(0.05))
        if E.truthy(E.op_ne(E.gv(60), 3)) then
          E.load_var(o, 93)
          E.fb_set(E.fb_key(E.gv(61), E.gv(62), "В"), E.gv(93), "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 21)) end,
      body = function(o)
        E.add_gv(93, 3)
        E.store_var(o, E.gv(93), 93)
        E.set_pos(o, E.random(E.op_neg(600), 600), E.random(E.op_neg(300), 300))
      end,
    },
  },
}
