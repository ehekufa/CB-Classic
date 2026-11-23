-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 91 "Дед мороз лаки блок" / объект 333 "Дед мороз лаки блок"
local E = require("runtime.engine")

return {
  scene_id = 91,
  object_id = 333,
  name = "Дед мороз лаки блок",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.wait(E.num(0.1))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        for _ = 1, math.floor(E.num(25)) do
          E.add_size(o, E.op_neg(5))
          E.frame()
        end
        E.set_image_by_id(o, 512)
        for _ = 1, math.floor(E.num(25)) do
          E.add_size(o, 6)
          E.frame()
        end
        if E.truthy(E.op_eq(E.random(1, 100), 1)) then
          E.add_gv(24, 7777)
          E.toast("СЛАДОСТЬ! Вам дали 7777🏆!")
          E.toast("Даже разработчик офигел!")
        else
          E.toast("ГАДОСТЬ! Вам на дали кубки!")
        end
        E.wait(E.num(0.2))
        for _ = 1, math.floor(E.num(22)) do
          E.add_size(o, E.op_neg(7))
          E.frame()
        end
        E.run_scene(31) return
      end,
    },
  },
}
