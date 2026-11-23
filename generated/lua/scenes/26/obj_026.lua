-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 26 "Супер атака"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 26,
  name = "Супер атака",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.camera_off()
        E.set_pos(o, 440, E.op_neg(45))
        E.set_size(o, 52)
      end,
    },
    { -- скрипт 2: function:16
      trigger = "function:16",
      body = function(o)
        E.hide(o)
        E.wait(E.num(3.05))
        E.show(o)
      end,
    },
    { -- скрипт 3: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.bright(o), 100)) then
          E.set_bright(o, 85)
          if E.truthy(E.op_eq(E.gv(47), 1)) then
            E.set_gv(65, 1)
            E.broadcast_to_object(21, 8)
            E.broadcast_to_object(25, 8)
            E.broadcast_to_object(555, 8)
            E.wait(E.num(10))
          end
          if E.truthy(E.op_eq(E.gv(47), 2)) then
            E.clone(247)
            E.wait(E.num(5.5))
          end
          if E.truthy(E.op_eq(E.gv(47), 3)) then
            E.broadcast_to_object(418, 13)
            E.wait(E.num(2))
          end
          if E.truthy(E.op_eq(E.gv(47), 4)) then
            E.play_sound(o, 53)
            E.set_gv(85, 1)
            E.wait(E.num(1.5))
            E.set_gv(85, 0)
            E.wait(E.num(4.2))
          end
          if E.truthy(E.op_eq(E.gv(47), 5)) then
            E.clone(412)
            E.wait(E.num(1.1))
          end
          if E.truthy(E.op_eq(E.gv(47), 6)) then
            E.broadcast_to_object(21, 5)
            E.wait(E.num(2.1))
          end
          E.set_bright(o, 100)
        end
      end,
    },
    { -- скрипт 4: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(68), true)) end,
      body = function(o)
        E.wait_until(function() return E.truthy(E.op_eq(E.gv(68), true)) end, o)
        E.set_bright(o, 85)
      end,
    },
    { -- скрипт 5: start
      trigger = "start",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(47), 6)) then
          E.play_sound(o, 53)
          E.set_gv(85, 1)
          E.wait(E.num(1.5))
          E.set_gv(85, 0)
          E.wait(E.num(4.2))
        end
      end,
    },
  },
}
