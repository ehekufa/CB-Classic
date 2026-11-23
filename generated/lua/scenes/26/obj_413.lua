-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 413 "Снежинка"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 413,
  name = "Снежинка",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 450)
        E.hide(o)
        while true do
          E.clone_self(o)
          E.wait(E.num(0.18))
          E.frame()
        end
      end,
    },
    { -- скрипт 2: clone
      trigger = "clone",
      body = function(o)
        E.set_x(o, E.random(E.op_neg(700), 700))
        E.show(o)
        E.transition(o, 2.2, E.pos_x(o), E.op_neg(450))
        E.delete_clone(o) return
      end,
    },
  },
}
