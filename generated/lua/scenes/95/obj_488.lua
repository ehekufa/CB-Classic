-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 488 "Кубик (Дождь)"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 488,
  name = "Кубик (Дождь)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 350)
        E.set_image_by_id(o, 838)
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
    { -- скрипт 4: function:19
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
    { -- скрипт 5: function:24
      trigger = "function:24",
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
    { -- скрипт 7: function:15
      trigger = "function:15",
      body = function(o)
        E.wait(E.num(1))
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(1))
          E.clone_self(o)
          E.wait(E.num(1))
        end
      end,
    },
    { -- скрипт 8: clone
      trigger = "clone",
      body = function(o)
        E.show(o)
        E.set_image_by_id(o, 839)
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
