-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 24 "Джойстик"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 24,
  name = "Джойстик",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(590), E.op_neg(150))
        E.camera_off()
        E.set_rot(o, E.op_neg(90))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(7, E.touch_count())
        E.broadcast_to_object(21, 2)
        E.set_gv(79, true)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.set_rot(o, E.op_add(E.atan2deg(E.op_sub(E.pos_x(o), E.touch_x_by_id(E.gv(7))), E.op_sub(E.pos_y(o), E.touch_y_by_id(E.gv(7)))), 90))
          E.set_gv(2, E.op_add(E.rot(o), 90))
          E.frame()
        end
        E.set_rot(o, E.op_neg(90))
        E.set_gv(79, 0)
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(69), 0)) end,
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(69), 1)) end,
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 5: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(79), true)) end,
      body = function(o)
        while E.truthy(E.op_ne(E.gv(79), 0)) do
          E.play_sound_wait(o, 14)
          E.frame()
        end
      end,
    },
  },
}
