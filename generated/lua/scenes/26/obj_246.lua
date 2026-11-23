-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 246 "Пушка"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 246,
  name = "Пушка",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(47), 2)) then
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
          E.set_rot(o, E.gv(2))
          E.go_steps(o, 15)
          E.frame()
        end
      end,
    },
  },
}
