-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 497 "Джойстик"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 497,
  name = "Джойстик",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(590), E.op_neg(150))
        E.camera_off(o)
        E.set_rot(o, E.op_neg(90))
        E.ask(o, "Выберите Админа (1/3)", function(answer) E.set_gv(90, answer) end)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(7, E.touch_count())
        if E.truthy(E.op_eq(E.gv(90), "1")) then
          E.broadcast_to_object(504, 2)
        end
        if E.truthy(E.op_eq(E.gv(90), "2")) then
          E.broadcast_to_object(505, 2)
        end
        if E.truthy(E.op_eq(E.gv(90), "3")) then
          E.broadcast_to_object(524, 2)
        end
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.set_rot(o, E.op_add(E.atan2deg(E.op_sub(E.pos_x(o), E.touch_x_by_id(E.gv(7))), E.op_sub(E.pos_y(o), E.touch_y_by_id(E.gv(7)))), 90))
          E.set_gv(2, E.op_add(E.rot(o), 90))
          E.frame()
        end
        E.set_rot(o, E.op_neg(90))
      end,
    },
  },
}
