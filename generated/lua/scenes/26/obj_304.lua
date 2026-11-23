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
        E.camera_off()
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
        E.call_function(11)
        E.show(o)
        E.call_function(12)
        E.hide(o)
      end,
    },
  },
}
