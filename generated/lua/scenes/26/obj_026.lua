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
        if E.truthy(E.op_eq(E.gv(47), 7)) then
          E.hide(o)
        end
        if E.truthy(E.op_eq(E.gv(47), 0)) then
          E.hide(o)
        end
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.bright(o), 100)) then
          E.set_bright(o, 85)
          if E.truthy(E.op_eq(E.gv(47), 1)) then
            E.set_gv(65, 1)
            E.broadcast_to_object(21, 8)
            while true do
              E.set_bright(o, 85)
              E.frame()
            end
          end
          if E.truthy(E.op_eq(E.gv(47), 2)) then
            E.clone(247)
            E.wait(E.num(5.5))
          end
          if E.truthy(E.op_eq(E.gv(47), 3)) then
            E.clone(412)
            E.wait(E.num(0.18))
          end
          if E.truthy(E.op_eq(E.gv(47), 4)) then
            E.broadcast_to_object(418, 13)
            E.wait(E.num(2))
          end
          if E.truthy(E.op_eq(E.gv(47), 5)) then
            E.broadcast_to_object(21, 5)
            E.wait(E.num(2.1))
          end
          while true do
            E.set_bright(o, 100)
            E.frame()
          end
        end
      end,
    },
    { -- скрипт 3: when_truth
      trigger = "when_truth",
      cond = function(o) return E.truthy(E.op_eq(E.gv(68), true)) end,
      body = function(o)
        E.wait_until(function() return E.truthy(E.op_eq(E.gv(68), true)) end, o)
        while true do
          E.set_bright(o, 85)
          E.frame()
        end
      end,
    },
  },
}
