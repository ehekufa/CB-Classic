-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 101 "Конец" / объект 516 "Диалог"
local E = require("runtime.engine")

return {
  scene_id = 101,
  object_id = 516,
  name = "Диалог",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, E.op_neg(250))
        E.set_alpha(o, 10)
        E.hide(o)
        E.wait(E.num(2.8))
        E.show(o)
        E.wait(E.num(1.3))
        E.wait_until(function() return E.truthy(E.unknown_expr("touchDisplay")) end, o)
        E.set_image_by_id(o, 917)
        E.wait(E.num(1.3))
        E.wait_until(function() return E.truthy(E.unknown_expr("touchDisplay")) end, o)
        E.hide(o)
        E.wait(E.num(1.1))
        E.run_scene(102) return
      end,
    },
  },
}
