-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 96 "Достижения" / объект 582 "Saturday's Sacrifice"
local E = require("runtime.engine")

return {
  scene_id = 96,
  object_id = 582,
  name = "Saturday's Sacrifice",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_sub(25, 125))
        if E.truthy((not E.truthy(E.array_contains(E.array(1), "Saturday17th")))) then
          E.set_bright(o, 65)
        end
      end,
    },
    { -- скрипт 2: function:2
      trigger = "function:2",
      body = function(o)
        E.add_y(o, E.gv(48))
      end,
    },
  },
}
