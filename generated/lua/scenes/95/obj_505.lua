-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 505 "Колонки"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 505,
  name = "Колонки",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.hide(o)
        E.forever(function()
          E.wait(E.num(0.1))
          if E.truthy(E.op_eq(E.gv(82), "2")) then
            E.show(o)
          else
            E.hide(o)
          end
        end)
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.set_size(o, 220)
          E.wait(E.num(0.2))
          E.set_size(o, 230)
          E.wait(E.num(0.2))
        end)
      end,
    },
  },
}
