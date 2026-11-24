-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 26 "Игра" / объект 594 "Изумруд"
local E = require("runtime.engine")

return {
  scene_id = 26,
  object_id = 594,
  name = "Изумруд",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, E.random(E.op_neg(580), 580), E.random(E.op_neg(245), 245))
        E.hide(o)
        if E.truthy(E.op_eq(E.random(1, 12), 1)) then
          E.add_body(o, "static")
          E.set_sensor(o, "off")
          E.show(o)
          E.wait_until(function() return E.truthy(E.touching_object(o, 21)) end, o)
          E.add_gv(93, 1)
          E.delete_clone(o) return
        end
      end,
    },
  },
}
