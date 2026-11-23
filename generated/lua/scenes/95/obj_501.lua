-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 501 "Админ панель"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 501,
  name = "Админ панель",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(600), 160)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(71), 1)) then
          E.set_gv(71, 0)
        else
          E.set_gv(71, 1)
        end
      end,
    },
  },
}
