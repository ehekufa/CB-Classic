-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 247 "Пуля"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 247,
  name = "Пуля",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 180)
        E.hide(o)
      end,
    },
    { -- скрипт 2: clone
      trigger = "clone",
      body = function(o)
        E.go_to(o, 21)
        E.set_rot(o, E.gv(2))
        E.add_body(o, "dynamic")
        E.set_sensor(o, "off")
        E.show(o)
        for _ = 1, math.floor(E.num(100)) do
          E.go_steps(o, 12)
          E.frame()
        end
        E.delete_clone(o) return
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 22)) end,
      body = function(o)
        E.clone(19)
        E.wait(E.num(0.05))
        E.delete_clone(o) return
      end,
    },
  },
}
