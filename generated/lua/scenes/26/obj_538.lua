-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 538 "Интро"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 538,
  name = "Интро",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(28), 1)) then
          E.video_play(2)
          E.broadcast_to_object(26, 16)
          E.broadcast_to_object(25, 16)
          E.broadcast_to_object(24, 16)
          E.wait(E.num(3.05))
          E.video_stop(2)
          E.broadcast_to_object(25, 19)
          E.broadcast_to_object(26, 17)
        end
        if E.truthy(E.op_eq(E.gv(28), 2)) then
          E.video_play(3)
          E.broadcast_to_object(26, 16)
          E.broadcast_to_object(25, 18)
          E.wait(E.num(3.05))
          E.video_stop(3)
          E.broadcast_to_object(25, 19)
          E.broadcast_to_object(26, 17)
        end
      end,
    },
    { -- скрипт 2: function:14
      trigger = "function:14",
      body = function(o)
        if E.truthy(E.op_eq(E.gv(28), 1)) then
          E.video_stop(2)
        end
        if E.truthy(E.op_eq(E.gv(28), 2)) then
          E.video_stop(3)
        end
      end,
    },
  },
}
