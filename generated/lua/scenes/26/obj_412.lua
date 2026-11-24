-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 412 "Яйцо-мина"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 412,
  name = "Яйцо-мина",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 2: clone
      trigger = "clone",
      body = function(o)
        E.set_image_by_id(o, E.random(1, 2))
        E.go_to(o, 21)
        E.wait(E.num(0.03))
        E.add_body(o, "dynamic")
        E.set_sensor(o, "off")
        E.show(o)
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 22)) end,
      body = function(o)
        E.play_sound(o, 63)
        E.broadcast_to_object(22, 3)
        E.add_gv(4, E.op_neg(2))
        E.clone(19)
        E.delete_clone(o) return
      end,
    },
  },
}
