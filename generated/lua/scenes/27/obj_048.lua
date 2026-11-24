-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 27 "Кубы" / объект 48 "Скролл"
local E = require("runtime.engine")

return {
  scene_id = 27,
  object_id = 48,
  name = "Скролл",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 130, 10)
        E.set_size(o, 125)
        E.camera_off(o)
        E.clone_self(o)
        E.add_y(o, E.op_neg(140))
        E.set_image_by_id(o, 55)
        E.set_rot(o, 180)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.broadcast_to_object(49, 3)
        else
          E.broadcast_to_object(49, 2)
        end
      end,
    },
  },
}
