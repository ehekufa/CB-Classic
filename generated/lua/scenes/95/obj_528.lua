-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 528 "Открыть магазин скинов"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 528,
  name = "Открыть магазин скинов",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 57.699951171875, 179.8116070925)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.next_image(o)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.broadcast(22)
        else
          E.broadcast(23)
        end
      end,
    },
  },
}
