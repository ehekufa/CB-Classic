-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 500 "Ивент (кнопка)"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 500,
  name = "Ивент (кнопка)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_bright(o, 120)
        E.camera_off()
        E.set_size(o, 30)
        E.forever(function()
          E.go_to(o, 498)
          E.add_y(o, 0)
        end)
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        E.forever(function()
          E.fb_get("Ивент", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(85, v) end)
          if E.truthy(E.op_eq(E.gv(85), "3")) then
            E.broadcast(20)
          end
          if E.truthy(E.op_eq(E.gv(85), "2")) then
            E.broadcast(19)
          end
          if E.truthy(E.op_eq(E.gv(85), "0")) then
            E.broadcast(16)
          end
          if E.truthy(E.op_eq(E.gv(85), "1")) then
            E.broadcast(15)
          end
          E.wait(E.num(0.05))
        end)
      end,
    },
    { -- скрипт 3: function:17
      trigger = "function:17",
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 4: function:18
      trigger = "function:18",
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 5: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 130)
        E.ask(o, "Какой ивент??", function(answer) E.set_gv(84, answer) end)
      end,
    },
    { -- скрипт 6: tap
      trigger = "tap",
      body = function(o)
        E.set_bright(o, 120)
      end,
    },
    { -- скрипт 7: function:14
      trigger = "function:14",
      body = function(o)
        E.fb_set("Ивент", E.gv(84), "https://cubic-battle-default-rtdb.firebaseio.com/")
      end,
    },
  },
}
