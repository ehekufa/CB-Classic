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
        if E.truthy(E.op_eq(E.image_number(o), 3)) then
          E.set_size(o, 120)
        end
        while true do
          E.set_gravity(0, 0)
          E.text_show_var(3, E.pos_x(o), E.op_add(E.pos_y(o), 125), 185, "#FF0000", "center")
          E.frame()
        end
        E.call_function(2)
        while E.truthy(E.touch_id_by_index(E.gv(7))) do
          if E.truthy(E.op_eq(E.gv(47), 7)) then
            E.go_steps(o, 80)
            E.set_rot(o, E.gv(2))
            E.wait(E.num(0.3))
          else
            E.go_steps(o, 8)
            E.set_rot(o, E.gv(2))
          end
          E.frame()
        end
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
        E.call_function(8)
        for _ = 1, math.floor(E.num(8)) do
          E.go_steps(o, 27)
          if E.truthy(E.touching_object(o, 22)) then
            E.set_gv(68, true)
            E.add_gv(4, E.op_neg(1))
          end
          E.frame()
        end
        E.go_steps(o, E.op_neg(210))
        E.set_gv(65, 0)
      end,
    },
    { -- скрипт 3: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.09))
        if E.truthy(E.op_eq(E.image_number(o), 12)) then
          E.particles_create(o, "Частицы", 884)
          while true do
            E.particles_move(o, "Частицы", E.pos_x(o), E.pos_y(o))
            E.frame()
          end
        end
      end,
    },
    { -- скрипт 4: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.09))
        if E.truthy(E.op_eq(E.image_number(o), 11)) then
          E.particles_create(o, "Частицы", 884)
          while true do
            E.particles_move(o, "Частицы", E.pos_x(o), E.pos_y(o))
            E.frame()
          end
        end
      end,
    },
    { -- скрипт 5: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.09))
        if E.truthy(E.op_eq(E.image_number(o), 9)) then
          E.particles_create(o, "Частицы", 506)
          while true do
            E.particles_move(o, "Частицы", E.pos_x(o), E.pos_y(o))
            E.frame()
          end
        end
      end,
    },
    { -- скрипт 6: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.05))
        if E.truthy(E.op_eq(E.gv(47), 5)) then
          E.set_image_by_id(o, 715)
          E.set_size(o, 100)
          E.set_hitbox(o, 150, 150)
        end
      end,
    },
    { -- скрипт 7: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.05))
        if E.truthy(E.op_eq(E.gv(47), 5)) then
          E.set_image_by_id(o, 715)
          E.set_size(o, 100)
          E.set_hitbox(o, 150, 150)
        end
        E.call_function(5)
        E.set_gv(65, 1)
        E.set_sensor(o, "off")
        for _ = 1, math.floor(E.num(95)) do
          E.go_steps(o, 20)
          if E.truthy(E.touching_object(o, 22)) then
            E.add_gv(4, E.op_neg(0.3))
          end
          E.frame()
        end
        E.set_gv(65, 0)
        E.set_gv(4, E.round(E.gv(4)))
        E.set_sensor(o, "on")
      end,
    },
    { -- скрипт 8: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.09))
        if E.truthy(E.op_eq(E.image_number(o), 9)) then
          E.particles_create(o, "Частицы", 506)
          while true do
            E.particles_move(o, "Частицы", E.pos_x(o), E.pos_y(o))
            E.frame()
          end
        end
      end,
    },
  },
}
