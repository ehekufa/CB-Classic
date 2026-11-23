-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 42 "Загрузка" / объект 1 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 42,
  object_id = 1,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.adaptive_off()
        E.set_bg_color("#000000")
        E.fb_set("ScreenWidth", E.unknown_expr("displayActualWidth"), E.gv(56))
        E.fb_set("ScreenHeight", E.unknown_expr("displayActualHeight"), E.gv(56))
        E.set_gv(56, "https://cubicbattleserver-19ae2-default-rtdb.firebaseio.com/")
        E.load_var(o, 24)
        E.set_gv(12, 1)
        if E.truthy(E.op_gt(E.gv(24), 1)) then
          E.load_var(o, 80)
          E.load_var(o, 47)
          E.load_var(o, 93)
          E.unsupported("readArray")
          E.unsupported("readArray")
        else
          E.toast("Добро пожаловать!")
        end
      end,
    },
    { -- скрипт 2: back
      trigger = "back",
      body = function(o)
        E.exit_game()
      end,
    },
  },
}
