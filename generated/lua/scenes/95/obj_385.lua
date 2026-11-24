-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 385 "Фон"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 385,
  name = "Фон",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_gravity(0, 0)
        E.adaptive_off()
        E.set_gv(8, 0)
        E.set_gv(2, 0)
        E.set_size(o, 105)
        E.forever(function()
          E.wait(E.num(0.1))
          if E.truthy(E.op_eq(E.gv(82), "3")) then
            E.set_image_by_id(o, 1133)
          else
            E.set_image_by_id(o, 614)
          end
        end)
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
