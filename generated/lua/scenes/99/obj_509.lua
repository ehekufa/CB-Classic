-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 509 "Кубик (Дождь) (1)"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 509,
  name = "Кубик (Дождь) (1)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.load_var(o, E.gv(93))
        E.set_size(o, 50)
        E.set_image_by_id(o, 880)
        E.add_body(o, "dynamic")
        E.hide(o)
      end,
    },
    { -- скрипт 2: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.truthy(E.op_eq(E.gv(90), "2")) and E.truthy(E.touching_object(o, 505))) end,
      body = function(o)
        E.add_gv(93, 5)
        E.store_var(o, E.gv(93))
        if E.truthy(E.op_ne(E.gv(60), 3)) then
          E.load_var(o, E.gv(2))
          E.fb_set(E.fb_key(E.gv(61), E.gv(62), "В"), E.gv(93), "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.truthy(E.op_eq(E.gv(90), "1")) and E.truthy(E.touching_object(o, 504))) end,
      body = function(o)
        E.add_gv(93, 5)
        E.store_var(o, E.gv(93))
        if E.truthy(E.op_ne(E.gv(60), 3)) then
          E.load_var(o, E.gv(2))
          E.fb_set(E.fb_key(E.gv(61), E.gv(62), "В"), E.gv(93), "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
      end,
    },
    { -- скрипт 4: start
      trigger = "start",
      body = function(o)
        while true do
          E.add_rot(o, E.random(1, 50))
          E.frame()
        end
      end,
    },
    { -- скрипт 5: start
      trigger = "start",
      body = function(o)
        while true do
          E.transition(o, 1, E.random(E.op_neg(300), 300), E.random(E.op_neg(300), 300))
          E.frame()
        end
        E.call_function(25)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(20)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(15)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(16)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.wait(E.num(1))
          E.hide(o)
          E.wait(E.num(1))
        end
        E.call_function(19)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.show(o)
        end
      end,
    },
  },
}
