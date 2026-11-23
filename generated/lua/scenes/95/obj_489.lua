-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 489 "Фон мод меню"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 489,
  name = "Фон мод меню",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 140)
      end,
    },
    { -- скрипт 2: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(71), 0)) end,
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(71), 1)) end,
      body = function(o)
        E.show(o)
      end,
    },
  },
}
