-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 281 "Конфета"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 281,
  name = "Конфета",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 560, E.op_neg(195))
        E.set_size(o, 80)
        while true do
          E.fb_get(E.fb_key(E.gv(61), E.gv(62), "В"), "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(92, v) end)
          E.text_show_var(92, E.pos_x(o), E.op_add(E.pos_y(o), 160), 110, "#FFFFFF", "center")
          if E.truthy(E.op_ge(E.gv(93), 100000)) then
            E.set_lv(o, 1, "ban")
            E.store_var(o, E.lv(o, 1))
          else
            E.set_lv(o, 1, "unban")
            E.store_var(o, E.lv(o, 1))
          end
          E.wait(E.num(0.5))
          E.frame()
        end
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
        E.run_scene(89) return
      end,
    },
  },
}
