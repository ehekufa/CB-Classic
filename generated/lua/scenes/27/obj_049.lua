-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 27 "Кубы" / объект 49 "Камера"
local E = require("runtime.engine")

return {
  scene_id = 27,
  object_id = 49,
  name = "Камера",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.camera_focus(0, 0)
        E.call_function(3)
        E.add_y(o, 80)
        E.call_function(2)
        E.add_y(o, E.op_neg(80))
      end,
    },
  },
}
