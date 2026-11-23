-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 31 "Больше" / объект 424 "Ввести"
local E = require("runtime.engine")

return {
  scene_id = 31,
  object_id = 424,
  name = "Ввести",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 400, E.op_neg(230))
        E.text_create("Промокод", "off", 400, 100, 81)
        E.text_set("Промокод", 'pos', E.pos_x(o), E.op_add(E.pos_y(o), 130))
        E.unsupported("setFontSizeTextField")
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
        if E.truthy(E.truthy(E.op_eq(E.gv(81), "MurderOfLove")) and E.truthy((not E.truthy(E.array_contains(E.array(2), "MurderOfLove"))))) then
          E.add_gv(24, 45)
          E.array_add(2, "MurderOfLove")
        end
        if E.truthy(E.truthy(E.op_eq(E.gv(81), "Eggs")) and E.truthy((not E.truthy(E.array_contains(E.array(2), "Eggs"))))) then
          E.array_add(2, "Eggs")
          E.broadcast_to_object(-1, 2)
          E.wait(E.num(0.05))
          E.run_scene(104) return
        end
        if E.truthy(E.truthy(E.op_eq(E.gv(81), "GoldenEgg")) and E.truthy((not E.truthy(E.array_contains(E.array(2), "GoldenEgg"))))) then
          E.array_add(2, "GoldenEgg")
          E.broadcast_to_object(-1, 2)
          E.wait(E.num(0.05))
          E.run_scene(105) return
        end
        if E.truthy(E.truthy(E.op_eq(E.gv(81), "GoldenEgg")) and E.truthy((not E.truthy(E.array_contains(E.array(2), "Admin"))))) then
          E.array_add(2, "GoldenEgg")
          E.broadcast_to_object(-1, 2)
          E.wait(E.num(0.05))
          E.run_scene(105) return
        end
      end,
    },
    { -- скрипт 4: function:2
      trigger = "function:2",
      body = function(o)
        E.text_delete("Промокод")
      end,
    },
  },
}
