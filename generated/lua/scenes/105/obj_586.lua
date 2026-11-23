-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 105 "GoldenEgg" / объект 586 "Яйцо"
local E = require("runtime.engine")

return {
  scene_id = 105,
  object_id = 586,
  name = "Яйцо",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 150)
        E.wait(E.num(0.25))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        E.set_image_by_id(o, 1102)
        E.wait(E.num(0.25))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        E.set_image_by_id(o, 1104)
        E.wait(E.num(0.25))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        for _ = 1, math.floor(E.num(15)) do
          E.add_alpha(o, 10)
          E.frame()
        end
        E.set_lv(o, 1, E.random(1, 4))
        if E.truthy(E.truthy(E.op_eq(E.lv(o, 1), 1)) or E.truthy(E.op_eq(E.lv(o, 1), 2))) then
          E.toast("Вам выпало... 15 яиц!")
          E.add_gv(80, 15)
        end
        if E.truthy(E.op_eq(E.lv(o, 1), 3)) then
          for _ = 1, math.floor(E.num(2)) do
            E.array_add(1, "EasterEggBattle")
            E.frame()
          end
          E.toast("Вам выпал... Заяц куб!")
        end
        if E.truthy(E.op_eq(E.lv(o, 1), 4)) then
          for _ = 1, math.floor(E.num(2)) do
            E.array_add(1, "EasterClass")
            E.frame()
          end
          E.toast("Вам выпал... Пасхальник!")
        end
        E.wait(E.num(0.7))
        E.wait_until(function() return E.truthy(E.unknown_expr("touchDisplay")) end, o)
        E.run_scene(89) return
      end,
    },
  },
}
