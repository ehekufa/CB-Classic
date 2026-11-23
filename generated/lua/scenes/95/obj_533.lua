-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 533 "Кубик (Дождь) (2)"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 533,
  name = "Кубик (Дождь) (2)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_image_by_id(o, 944)
        E.hide(o)
        E.call_function(25)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(15)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(19)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(16)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(24)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(0.5))
          E.clone_self(o)
          E.wait(E.num(0.5))
        end
      end,
    },
    { -- скрипт 2: clone
      trigger = "clone",
      body = function(o)
        E.show(o)
        E.set_image_by_id(o, 946)
        for _ = 1, math.floor(E.num(1)) do
          E.set_pos(o, E.random(E.op_neg(600), 600), 300)
          E.transition(o, 5, E.pos_x(o), E.op_neg(1000))
          E.wait(E.num(1))
          E.set_pos(o, E.random(E.op_neg(600), 600), 1360)
          E.wait(E.num(1))
          E.frame()
        end
        E.delete_clone(o) return
        E.hide(o)
      end,
    },
  },
}
