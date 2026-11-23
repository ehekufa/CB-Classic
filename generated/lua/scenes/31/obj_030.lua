-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 31 "Больше" / объект 30 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 31,
  object_id = 30,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.adaptive_off()
      end,
    },
    { -- скрипт 2: back
      trigger = "back",
      body = function(o)
        E.broadcast_to_object(424, 2)
        E.wait(E.num(0.06))
        E.run_scene(1) return
      end,
    },
  },
}
