-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 539 "Эмоции"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 539,
  name = "Эмоции",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, -642, 218.74999523163)
        E.forever(function()
          E.fb_get("1", E.gv(56), function(v) E.set_gv(87, v) end)
          E.fb_get("2", E.gv(56), function(v) E.set_gv(88, v) end)
          E.fb_get("3", E.gv(56), function(v) E.set_gv(89, v) end)
        end)
      end,
    },
    { -- скрипт 2: touch_down
      trigger = "touch_down",
      body = function(o)
        if E.truthy(E.op_eq(E.image_number(o), 1)) then
          E.fb_set(E.gv(72), "B-Hop", E.gv(56))
          E.set_image_by_id(o, 967)
          E.toast("Эмоция BHop включена")
        else
          E.fb_set(E.gv(72), "None", E.gv(56))
          E.set_image_by_id(o, 966)
          E.toast("Эмоция BHop выключена")
        end
      end,
    },
  },
}
