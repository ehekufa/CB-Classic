-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 77 "Регистрация" / объект 166 "Зарегистрироваться"
local E = require("runtime.engine")

return {
  scene_id = 77,
  object_id = 166,
  name = "Зарегистрироваться",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(380), E.op_neg(50))
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(60, E.image_name(o))
        E.set_bright(o, 120)
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_ne(E.gv(60), E.image_name(o))) end,
      body = function(o)
        E.set_bright(o, 100)
      end,
    },
  },
}
