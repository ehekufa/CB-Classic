-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 42 "Загрузка" / объект 186 "Спасибо"
local E = require("runtime.engine")

return {
  scene_id = 42,
  object_id = 186,
  name = "Спасибо",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_y(o, 205)
        E.set_alpha(o, 100)
        E.set_gv(47, 0)
        E.wait(E.num(0.3))
        for _ = 1, math.floor(E.num(29)) do
          E.add_alpha(o, E.op_neg(3.8))
          E.frame()
        end
        E.wait(E.num(0.5))
        for _ = 1, math.floor(E.num(29)) do
          E.add_alpha(o, 3.8)
          E.frame()
        end
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        while true do
          E.add_size(o, 0.2)
          E.frame()
        end
      end,
    },
  },
}
