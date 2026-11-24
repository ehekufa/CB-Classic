-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 303 "Mod-menu"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 303,
  name = "Mod-menu",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(595), 160)
        E.camera_off(o)
        if E.truthy(E.op_eq(E.gv(71), true)) then
          E.show(o)
        end
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.wait(E.num(0.09))
        if E.truthy(E.touch_active()) then
          while E.truthy(E.touch_active()) do
            E.go_to_touch(o)
            E.frame()
          end
        else
          E.broadcast_to_object(304, 11)
          E.broadcast_to_object(309, 11)
          E.broadcast_to_object(305, 11)
        end
      end,
    },
  },
}
