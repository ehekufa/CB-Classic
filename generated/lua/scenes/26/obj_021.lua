-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 21 "Игрок"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 21,
  name = "Игрок",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 350)
        E.set_image_by_id(o, E.gv(12))
        E.set_gv(68, false)
        E.set_gv(3, 1)
        E.add_body(o, "dynamic")
        E.set_sensor(o, "off")
        E.set_hitbox(o, 115, 115)
        if E.truthy(E.op_eq(E.image_number(o), 3)) then
          E.set_size(o, 120)
        end
        E.forever(function()
          E.set_gravity(0, 0)
          E.text_show_var(3, E.pos_x(o), E.op_add(E.pos_y(o), 125), 185, "#FF0000", "center")
        end)
      end,
    },
    { -- скрипт 2: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 22)) end,
      body = function(o)
        if E.truthy(E.op_eq(E.gv(65), 0)) then
          if E.truthy(E.op_eq(E.gv(68), true)) then
            E.set_rot(o, E.gv(43))
            E.go_steps(o, 190)
            E.set_gv(68, 0)
          else
            E.run_scene(1) return
          end
        end
      end,
    },
    { -- скрипт 3: function:2
      trigger = "function:2",
      body = function(o)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          E.go_steps(o, 7)
          E.set_rot(o, E.gv(2))
          E.frame()
        end
      end,
    },
    { -- скрипт 4: function:8
      trigger = "function:8",
      body = function(o)
        E.wait(E.num(0.35))
        for _ = 1, math.floor(E.num(14)) do
          E.go_steps(o, 25)
          if E.truthy(E.touching_object(o, 22)) then
            E.set_gv(68, true)
            E.add_gv(4, E.op_neg(1))
            E.play_sound(o, 52)
            E.clone(19)
            E.set_gv(2, E.rot(o))
          end
          E.frame()
        end
        E.set_gv(65, 0)
      end,
    },
    { -- скрипт 5: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.05))
        if E.truthy(E.op_eq(E.gv(47), 6)) then
          E.set_image_by_id(o, 985)
        end
      end,
    },
    { -- скрипт 6: function:5
      trigger = "function:5",
      body = function(o)
        E.set_gv(65, 1)
        E.set_sensor(o, "off")
        E.broadcast_to_object(24, 15)
        for _ = 1, math.floor(E.num(150)) do
          E.go_steps(o, 3)
          if E.truthy(E.touching_object(o, 22)) then
            E.add_gv(4, 0.5)
            E.clone(19)
          end
          E.frame()
        end
        E.set_gv(65, 0)
        E.set_gv(4, E.round(E.gv(4)))
        E.set_sensor(o, "on")
      end,
    },
    { -- скрипт 7: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.wait(E.num(E.random(7, 12)))
          if E.truthy(E.op_eq(E.image_number(o), 7)) then
            if E.truthy(E.op_eq(E.random(1, 2), 1)) then
              E.play_sound(o, 58)
            else
              E.play_sound(o, 59)
            end
          end
          if E.truthy(E.op_eq(E.gv(47), 1)) then
            if E.truthy(E.op_eq(E.random(1, 2), 1)) then
              E.play_sound(o, 60)
            else
              E.play_sound(o, 61)
            end
          end
        end)
      end,
    },
  },
}
