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
    { -- скрипт 3: start
      trigger = "start",
      body = function(o)
        while true do
          E.fb_get("Написано", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(82, v) end)
          E.text_show_var(82, 100, 200, 120, "#FF0000", "center")
          E.wait(E.num(0.05))
          E.frame()
        end
      end,
    },
  },
}
