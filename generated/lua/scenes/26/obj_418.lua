-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 418 "Лазер"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 418,
  name = "Лазер",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 110)
        E.set_alpha(o, 100)
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        while true do
          E.go_to(o, 21)
          E.face(o, 22)
          E.go_steps(o, 320)
          E.frame()
        end
        E.call_function(13)
        E.add_body(o, "dynamic")
        E.set_sensor(o, "off")
        E.set_hitbox(o, 50, 450)
        E.set_alpha(o, 0)
        E.set_image_by_id(o, 702)
        E.wait(E.num(0.5))
        if E.truthy(E.op_ne(E.random(1, 5), 5)) then
          E.set_image_by_id(o, 699)
          E.play_sound(o, 20)
          for _ = 1, math.floor(E.num(2)) do
            if E.truthy(E.truthy(E.touching_object(o, 22)) and E.truthy(E.op_ne(E.op_sub(100, E.alpha(o)), 100))) then
              E.add_gv(4, E.op_neg(2))
            end
            E.frame()
          end
          E.set_gv(4, E.round(E.gv(4)))
          E.wait(E.num(0.4))
        else
          E.wait(E.num(0.1))
          E.set_image_by_id(o, 701)
          E.play_sound(o, 17)
          E.wait(E.num(0.7))
          E.run_scene(1) return
        end
        E.add_body(o, "noPhysic")
        E.set_alpha(o, 100)
      end,
    },
  },
}
