-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 410 "Игрок 3"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 410,
  name = "Игрок 3",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_sensor(o, "off")
        E.set_size(o, 350)
        E.forever(function()
          E.fb_get("X3", E.gv(56), function(v) E.set_gv(77, v) end)
          E.fb_get("Y3", E.gv(56), function(v) E.set_gv(78, v) end)
          E.fb_get("ROT3", E.gv(56), function(v) E.set_gv(92, v) end)
          E.set_pos(o, E.gv(77), E.gv(78))
          E.wait(E.num(0.13))
        end)
      end,
    },
    { -- скрипт 2: function:2
      trigger = "function:2",
      body = function(o)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.fb_set("X3", E.pos_x(o), E.gv(56))
          E.fb_set("Y3", E.pos_y(o), E.gv(56))
          E.fb_set("ROT3", E.rot(o), E.gv(56))
          E.set_pos(o, E.gv(77), E.gv(78))
          E.set_rot(o, E.gv(2))
          E.go_steps(o, 50)
          E.frame()
        end
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(89), "None")) end,
      body = function(o)
        E.set_image_by_id(o, 976)
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(89), "B-Hop")) end,
      body = function(o)
        E.set_image_by_id(o, 982)
        while E.truthy(E.op_eq(E.gv(89), "B-Hop")) do
          E.wait(E.num(0.25))
          E.set_size(o, 270)
          E.wait(E.num(0.25))
          E.set_size(o, 250)
          E.frame()
        end
      end,
    },
  },
}
