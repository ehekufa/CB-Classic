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
        E.set_gv(85, 0)
        E.set_size(o, 105)
        E.set_image_by_id(o, E.random(1, 2))
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
