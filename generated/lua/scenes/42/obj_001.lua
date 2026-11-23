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
        E.set_gv(12, 1)
        E.set_gv(46, 1)
        E.set_gv(56, "https://cubic-battle-default-rtdb.firebaseio.com/")
      end,
    },
    { -- скрипт 2: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(108), "ban")) end,
      body = function(o)
        E.exit_game()
      end,
    },
    { -- скрипт 3: back
      trigger = "back",
      body = function(o)
        E.exit_game()
      end,
    },
  },
}
