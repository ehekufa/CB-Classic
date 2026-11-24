-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 512 "Ивент"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 512,
  name = "Ивент",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 615, E.op_neg(220))
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        E.hide(o)
        E.set_size(o, 50)
        E.forever(function()
          E.wait(E.num(0.1))
          if E.truthy(E.op_ne(E.gv(82), "0")) then
            E.set_image_by_id(o, E.gv(82))
            E.show(o)
          else
            E.hide(o)
          end
        end)
      end,
    },
  },
}
