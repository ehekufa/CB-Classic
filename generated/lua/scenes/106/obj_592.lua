-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 106 "Плохая концовка" / объект 592 "???"
local E = require("runtime.engine")

return {
  scene_id = 106,
  object_id = 592,
  name = "???",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 150, 170)
        E.wait(E.num(0.1))
        E.transition(o, 1, E.pos_x(o), 60)
        E.play_sound(o, 66)
        E.wait(E.num(0.1))
        E.transition(o, 0.9, E.pos_x(o), 170)
        E.wait(E.num(0.1))
        E.transition(o, 1.1, E.op_neg(190), E.pos_y(o))
        E.wait(E.num(0.1))
        E.transition(o, 1.1, E.pos_x(o), 60)
        E.play_sound(o, 66)
        E.wait(E.num(0.1))
        E.transition(o, 1.1, E.pos_x(o), 170)
        E.wait(E.num(0.1))
        E.transition(o, 1, 72, E.pos_y(o))
        E.wait(E.num(0.1))
        E.transition(o, 1.1, E.pos_x(o), 60)
        E.play_sound(o, 66)
        E.wait(E.num(0.1))
        E.transition(o, 1.1, E.pos_x(o), 170)
        E.wait(E.num(0.1))
        E.transition(o, 1.1, 0, E.pos_y(o))
        E.wait(E.num(1.2))
        E.transition(o, 3.2, E.pos_x(o), 800)
      end,
    },
  },
}
