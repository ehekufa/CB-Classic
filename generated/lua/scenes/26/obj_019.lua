-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 19 "Кровь"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 19,
  name = "Кровь",
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
        E.go_to(o, 22)
        E.set_rot(o, E.random(0, 360))
        E.set_size(o, 0)
        E.set_layer(o, 3)
        E.show(o)
        for _ = 1, math.floor(E.num(18)) do
          E.add_size(o, 5)
          E.frame()
        end
        E.wait(E.num(9))
        for _ = 1, math.floor(E.num(10)) do
          E.add_size(o, E.op_neg(15))
          E.frame()
        end
        E.delete_clone(o) return
      end,
    },
  },
}
