-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 502 "Admin panel open"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 502,
  name = "Admin panel open",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, -514.69997406006, 262.37013191489)
        E.broadcast(17)
        E.camera_off(o)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        while E.truthy(E.touch_active()) do
          E.go_to_touch(o)
          E.frame()
        end
      end,
    },
    { -- скрипт 3: touch_down
      trigger = "touch_down",
      body = function(o)
        E.next_image(o)
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.broadcast(18)
        else
          E.broadcast(17)
        end
      end,
    },
  },
}
