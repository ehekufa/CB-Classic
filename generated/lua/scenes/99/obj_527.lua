-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 527 "Кубик (Дождь) (2)"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 527,
  name = "Кубик (Дождь) (2)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 100)
        E.set_image_by_id(o, 927)
        E.hide(o)
      end,
    },
    { -- скрипт 2: clone
      trigger = "clone",
      body = function(o)
        E.forever(function()
          E.add_rot(o, 15)
        end)
      end,
    },
    { -- скрипт 3: function:25
      trigger = "function:25",
      body = function(o)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
      end,
    },
    { -- скрипт 4: function:15
      trigger = "function:15",
      body = function(o)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
      end,
    },
    { -- скрипт 5: function:19
      trigger = "function:19",
      body = function(o)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
      end,
    },
    { -- скрипт 6: function:16
      trigger = "function:16",
      body = function(o)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 2)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
      end,
    },
    { -- скрипт 7: function:20
      trigger = "function:20",
      body = function(o)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(0.5))
          E.clone_self(o)
          E.wait(E.num(1))
        end
      end,
    },
    { -- скрипт 8: clone
      trigger = "clone",
      body = function(o)
        E.show(o)
        E.set_image_by_id(o, 943)
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
