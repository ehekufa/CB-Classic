-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 70 "Режимы" / объект 507 "Онлайн (1)"
local E = require("runtime.engine")

return {
  scene_id = 70,
  object_id = 507,
  name = "Онлайн (1)",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.op_neg(440), E.op_sub(E.op_sub(230, 95), 95))
        if E.truthy(E.truthy(E.truthy(E.op_eq(E.gv(61), "1111")) and E.truthy(E.op_eq(E.gv(62), "1111"))) or E.truthy(E.truthy(E.op_eq(E.gv(61), "Dimasi4ek229")) and E.truthy(E.op_eq(E.gv(62), "5252")))) then
          E.show(o)
        else
          E.hide(o)
        end
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 3: touch_up
      trigger = "touch_up",
      body = function(o)
        E.wait(E.num(0.04))
        E.set_bright(o, 100)
        E.wait(E.num(0.06))
        E.run_scene(99) return
      end,
    },
  },
}
