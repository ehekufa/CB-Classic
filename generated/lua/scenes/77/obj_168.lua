-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 77 "Регистрация" / объект 168 "Подтвердить"
local E = require("runtime.engine")

return {
  scene_id = 77,
  object_id = 168,
  name = "Подтвердить",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_neg(200))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_ne(E.gv(60), 0)) then
          E.text_delete("Имя")
          E.text_delete("Пароль")
          if E.truthy(E.op_eq(E.gv(60), "Войти")) then
            E.fb_get(E.fb_key(E.gv(61), E.gv(62)), "https://cubicbattleserver10-default-rtdb.firebaseio.com/", function(v) E.set_gv(24, v) end)
          else
            E.fb_set(E.fb_key(E.gv(61), E.gv(62)), 0, "https://cubicbattleserver10-default-rtdb.firebaseio.com/")
          end
          E.wait(E.num(0.09))
          E.run_scene(1) return
        else
          E.toast("Ошибка: вы ничего не выбрали")
        end
      end,
    },
  },
}
