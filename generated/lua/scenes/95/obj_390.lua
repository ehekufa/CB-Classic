-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 390 "Игрок 1"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 390,
  name = "Игрок 1",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.add_body(o, "static")
        E.set_sensor(o, "off")
        E.set_size(o, 350)
        E.forever(function()
          E.fb_get("X1", E.gv(56), function(v) E.set_gv(73, v) end)
          E.fb_get("Y1", E.gv(56), function(v) E.set_gv(74, v) end)
          E.fb_get("ROT1", E.gv(56), function(v) E.set_gv(90, v) end)
          E.set_pos(o, E.gv(73), E.gv(74))
          E.wait(E.num(0.13))
        end)
      end,
    },
    { -- скрипт 2: function:2
      trigger = "function:2",
      body = function(o)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.fb_set("X1", E.pos_x(o), E.gv(56))
          E.fb_set("Y1", E.pos_y(o), E.gv(56))
          E.fb_set("ROT1", E.rot(o), E.gv(56))
          E.set_pos(o, E.gv(73), E.gv(74))
          E.set_rot(o, E.gv(2))
          E.go_steps(o, 50)
          E.frame()
        end
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(87), "None")) end,
      body = function(o)
        E.set_image_by_id(o, 971)
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(87), "B-Hop")) end,
      body = function(o)
        E.set_image_by_id(o, 980)
        while E.truthy(E.op_eq(E.gv(87), "B-Hop")) do
          E.wait(E.num(0.25))
          E.set_size(o, 370)
          E.wait(E.num(0.25))
          E.set_size(o, 350)
          E.frame()
        end
      end,
    },
  },
}
