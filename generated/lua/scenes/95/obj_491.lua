-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 491 "Запустить ивент"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 491,
  name = "Запустить ивент",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_neg(90))
        E.forever(function()
          E.fb_get("Ивент", E.gv(56), function(v) E.set_gv(82, v) end)
          E.wait(E.num(1))
        end)
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
        E.ask(o, "Введите номер ивента", function(answer) E.set_gv(86, answer) E.call_function(13) end)
      end,
    },
    { -- скрипт 5: function:13
      trigger = "function:13",
      body = function(o)
        E.fb_set("Ивент", E.gv(86), E.gv(56))
      end,
    },
  },
}
