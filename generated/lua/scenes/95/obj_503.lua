-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 503 "Админское сообщение"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 503,
  name = "Админское сообщение",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.wait(E.num(0.9))
          E.fb_get("Админское сообщение", E.gv(56), function(v) E.set_gv(83, v) end)
        end)
      end,
    },
    { -- скрипт 2: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_ne(E.gv(83), "none")) end,
      body = function(o)
        E.text_show_var(83, 0, 125, 185, "#FF0000", "center")
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(83), "none")) end,
      body = function(o)
        E.cb.text_set("var:83", "visible", 0)
      end,
    },
  },
}
