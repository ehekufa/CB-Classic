-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 309 "Noclip"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 309,
  name = "Noclip",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(150), E.op_sub(220, 140))
        E.set_size(o, 80)
        E.hide(o)
        E.set_gv(65, 0)
        E.camera_off()
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(65), 0)) then
          E.set_gv(65, 1)
        else
          E.set_gv(65, 0)
        end
        E.call_function(11)
        E.show(o)
        E.call_function(12)
        E.hide(o)
      end,
    },
  },
}
