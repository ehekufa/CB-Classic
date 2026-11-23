-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 490 "Написать"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 490,
  name = "Написать",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 90)
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
    { -- скрипт 4: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_gv(71, 0)
        E.ask(o, "Что написать?", function(answer) E.set_gv(84, answer) end)
      end,
    },
    { -- скрипт 5: function:3
      trigger = "function:3",
      body = function(o)
        E.fb_set("Админское сообщение", E.gv(84), E.gv(56))
        E.wait(E.num(6.5))
        E.fb_set("Админское сообщение", "none", E.gv(56))
      end,
    },
  },
}
