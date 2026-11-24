-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 530 "скин влюблённого куба"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 530,
  name = "скин влюблённого куба",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 60, 50)
        E.hide(o)
      end,
    },
    { -- скрипт 2: function:22
      trigger = "function:22",
      body = function(o)
        E.show(o)
      end,
    },
    { -- скрипт 3: function:23
      trigger = "function:23",
      body = function(o)
        E.hide(o)
      end,
    },
    { -- скрипт 4: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 120)
      end,
    },
    { -- скрипт 5: touch_up
      trigger = "touch_up",
      body = function(o)
        E.set_bright(o, 100)
        if E.truthy(E.op_eq(E.gv(72), "2")) then
          E.fb_set("Какой скин у второго куба", "2", "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
        if E.truthy(E.op_eq(E.gv(72), "3")) then
          E.fb_set("какой скин у третьего куба", "2", "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
        if E.truthy(E.op_eq(E.gv(72), "1")) then
          E.fb_set("Какой скин у первого куба", "2", "https://cubic-battle-default-rtdb.firebaseio.com/")
        end
      end,
    },
  },
}
