-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 16 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 16,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.adaptive_off()
        E.set_gv(8, 0)
        E.set_gv(2, 0)
        if E.truthy(E.op_eq(E.image_name(o), "Фон")) then
          E.set_size(o, 105)
        else
          E.set_size(o, 150)
          E.set_bright(o, 130)
        end
      end,
    },
    { -- скрипт 2: back
      trigger = "back",
      body = function(o)
        E.run_scene(70) return
      end,
    },
  },
}
