-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 417 "Наш сервер"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 417,
  name = "Наш сервер",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 22, 181.97750602722)
        E.set_size(o, 150)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: tap
      trigger = "tap",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_bright(o, 100)
        E.wait(E.num(0.07))
        E.open_link("https://catrobat.org/")
      end,
    },
  },
}
