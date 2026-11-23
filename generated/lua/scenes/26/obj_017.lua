-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 17 "Тень игрока"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 17,
  name = "Тень игрока",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 350)
        E.set_bright(o, 0)
        E.set_alpha(o, 65)
        E.forever(function()
          E.go_to(o, 21)
          E.add_y(o, E.op_neg(28))
          E.set_rot(o, E.gv(2))
        end)
      end,
    },
  },
}
