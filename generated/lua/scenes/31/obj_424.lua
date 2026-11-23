-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 31 "Больше" / объект 424 "Ввести"
local E = require("runtime.engine")

return {
  scene_id = 31,
  object_id = 424,
  name = "Ввести",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.load_var(o, E.gv(93))
        E.set_pos(o, 400, E.op_neg(230))
        E.text_create("Промокод", "off", 400, 100, 81)
        E.text_set("Промокод", 'pos', E.pos_x(o), E.op_add(E.pos_y(o), 130))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: touch_up
      trigger = "touch_up",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_bright(o, 100)
        E.wait(E.num(0.06))
        if E.truthy(E.truthy(E.op_eq(E.gv(81), "Lover")) and E.truthy((not E.truthy(E.array_contains(E.array(2), "Lover"))))) then
          E.add_gv(93, 20)
          E.array_add(2, "Lover")
          E.store_var(o, E.gv(93))
          if E.truthy(E.op_ne(E.gv(60), 3)) then
            E.load_var(o, E.gv(93))
            E.fb_set(E.fb_key(E.gv(61), E.gv(62), "В"), E.gv(93), "https://cubic-battle-default-rtdb.firebaseio.com/")
          end
        else
          E.toast("этот промокод уже был введён!")
        end
        if E.truthy(E.truthy(E.op_eq(E.gv(81), "Chips Battle")) and E.truthy((not E.truthy(E.array_contains(E.array(2), "Lover"))))) then
          E.add_gv(80, 20)
          E.array_add(2, "Lover")
          E.store_var(o, E.gv(93))
          if E.truthy(E.op_ne(E.gv(60), 3)) then
            E.load_var(o, E.gv(93))
            E.fb_set(E.fb_key(E.gv(61), E.gv(62), "В"), E.gv(93), "https://cubic-battle-default-rtdb.firebaseio.com/")
          end
        else
          E.toast("этот промокод уже был введён!")
        end
      end,
    },
    { -- скрипт 4: function:2
      trigger = "function:2",
      body = function(o)
        E.text_delete("Промокод")
      end,
    },
  },
}
