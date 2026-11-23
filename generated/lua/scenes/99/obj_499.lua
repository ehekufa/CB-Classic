-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 499 "Написать"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 499,
  name = "Написать",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_bright(o, 120)
        E.camera_off()
        E.set_size(o, 30)
        while true do
          E.go_to(o, 498)
          E.add_y(o, 50)
          E.frame()
        end
      end,
    },
    { -- скрипт 2: start
      trigger = "start",
      body = function(o)
        while true do
          E.fb_get("Написано", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(82, v) end)
          E.text_show_var(82, 0, 300, 120, "#078707", "left")
          E.wait(E.num(0.05))
          E.frame()
        end
        E.call_function(17)
        E.hide(o)
        E.call_function(18)
        E.show(o)
      end,
    },
    { -- скрипт 3: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 130)
        E.call_function(21)
        E.ask(o, "Что написать??", function(answer) E.set_gv(83, answer) end)
      end,
    },
    { -- скрипт 4: tap
      trigger = "tap",
      body = function(o)
        E.set_bright(o, 120)
        E.broadcast(22)
        E.call_function(13)
        E.fb_set("Написано", E.fb_key(E.gv(94), E.gv(83)), "https://cubic-battle-default-rtdb.firebaseio.com/")
        E.wait(E.num(3))
        E.fb_set("Написано", "", "https://cubic-battle-default-rtdb.firebaseio.com/")
      end,
    },
  },
}
