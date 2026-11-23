-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 22 "Враг"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 22,
  name = "Враг",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(500), 200)
        E.set_size(o, 350)
        E.set_gv(4, 15)
        E.add_body(o, "static")
        E.forever(function()
          if E.truthy(E.op_eq(E.gv(69), 0)) then
            if E.truthy(E.op_eq(E.gv(47), 7)) then
              E.face(o, 21)
              E.go_steps(o, 1.1)
            else
              E.face(o, 21)
              E.go_steps(o, 3.3)
            end
          end
          E.set_gv(43, E.rot(o))
          E.text_show_var(4, E.pos_x(o), E.op_add(E.pos_y(o), 125), 185, "#FF0000", "center")
        end)
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        E.set_image_by_id(o, E.random(1, 2))
        E.forever(function()
          E.wait(E.num(0.1))
          if E.truthy(E.truthy(E.op_eq(E.gv(47), 1)) and E.truthy(E.op_eq(E.image_number(o), 2))) then
            E.toast("My... darling???")
            E.play_sound_wait(o, 30)
          end
          E.wait(E.num(0.1))
        end)
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.touching_object(o, 247)) end,
      body = function(o)
        E.go_steps(o, E.op_neg(100))
        E.add_gv(4, E.op_neg(5))
      end,
    },
    { -- скрипт 4: function:3
      trigger = "function:3",
      body = function(o)
        E.go_steps(o, E.op_neg(30))
      end,
    },
    { -- скрипт 5: function:9
      trigger = "function:9",
      body = function(o)
        while E.truthy(E.op_ne(E.gv(70), 0)) do
          E.set_rot(o, E.gv(70))
          E.go_steps(o, 3.92)
          E.frame()
        end
      end,
    },
  },
}
