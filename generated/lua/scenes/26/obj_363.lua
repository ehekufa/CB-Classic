-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 363 "Пушка (2)"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 363,
  name = "Пушка (2)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 120)
        E.wait(E.num(0.1))
        if E.truthy(E.op_eq(E.gv(47), 3)) then
          E.show(o)
        else
          E.hide(o)
        end
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        while true do
          E.go_to(o, 21)
          E.go_steps(o, 15)
          E.set_rot(o, E.gv(2))
          E.frame()
        end
      end,
    },
  },
}
