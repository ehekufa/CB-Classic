-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 520 "Сбросить прогресс"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 520,
  name = "Сбросить прогресс",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, -276.40003967285, -272.77183514813)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.ask(o, "кодовое слово (abc)", function(answer) E.set_gv(95, answer) end)
        E.call_function(5)
        if E.truthy(E.op_eq(E.gv(95), "abc")) then
          E.set_gv(93, 0)
          E.store_var(o, E.gv(93))
          if E.truthy(E.op_ne(E.gv(60), 3)) then
            E.fb_set(E.fb_key(E.gv(61), E.gv(62), "В"), E.gv(93), "https://cubic-battle-default-rtdb.firebaseio.com/")
            E.load_var(o, E.gv(93))
          end
          E.set_gv(24, 0)
          E.store_var(o, E.gv(24))
          if E.truthy(E.op_ne(E.gv(60), 3)) then
            E.fb_set(E.fb_key(E.gv(61), E.gv(62), "К"), E.gv(24), "https://cubic-battle-default-rtdb.firebaseio.com/")
            E.load_var(o, E.gv(24))
          end
        end
      end,
    },
  },
}
