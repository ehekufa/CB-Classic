-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 304 "Контроль"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 304,
  name = "Контроль",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(150), 220)
        E.set_size(o, 80)
        E.hide(o)
        E.camera_off(o)
        E.set_gv(69, 0)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(69), 0)) then
          E.set_gv(69, 1)
        else
          E.set_gv(69, 0)
        end
      end,
    },
    { -- скрипт 3: function:11
      trigger = "function:11",
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 4: function:12
      trigger = "function:12",
      body = function(o)
        E.hide(o)
      end,
    },
  },
}
