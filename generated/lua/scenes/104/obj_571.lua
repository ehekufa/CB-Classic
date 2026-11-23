-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 104 "Яйцо" / объект 571 "Яйцо"
local E = require("runtime.engine")

return {
  scene_id = 104,
  object_id = 571,
  name = "Яйцо",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 150)
        E.wait(E.num(0.25))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        E.set_image_by_id(o, 1054)
        E.wait(E.num(0.25))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        E.set_image_by_id(o, 1055)
        E.wait(E.num(0.25))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        for _ = 1, math.floor(E.num(15)) do
          E.add_alpha(o, 10)
          E.frame()
        end
        E.set_lv(o, 1, E.random(1, 10))
        if E.truthy(E.truthy(E.truthy(E.truthy(E.truthy(E.op_eq(E.lv(o, 1), 1)) or E.truthy(E.op_eq(E.lv(o, 1), 2))) or E.truthy(E.op_eq(E.lv(o, 1), 3))) or E.truthy(E.op_eq(E.lv(o, 1), 4))) or E.truthy(E.op_eq(E.lv(o, 1), 5))) then
          E.toast("Вам выпало... 1 Яйцо!")
          E.add_gv(80, 1)
        end
        if E.truthy(E.truthy(E.truthy(E.op_eq(E.lv(o, 1), 6)) or E.truthy(E.op_eq(E.lv(o, 1), 7))) or E.truthy(E.op_eq(E.lv(o, 1), 8))) then
          E.toast("Вам выпало... 2 Яйца!")
          E.add_gv(80, 2)
        end
        if E.truthy(E.op_eq(E.lv(o, 1), 9)) then
          for _ = 1, math.floor(E.num(2)) do
            E.array_add(1, "EasterEggBattle")
            E.frame()
          end
          E.toast("Вам выпал... Заяц куб!")
        end
        if E.truthy(E.op_eq(E.lv(o, 1), 10)) then
          for _ = 1, math.floor(E.num(2)) do
            E.array_add(1, "EasterClass")
            E.frame()
          end
          E.toast("Вам выпал... Пасхальник!")
        end
        E.wait(E.num(0.7))
        E.wait_until(function() return E.truthy(E.touch_active()) end, o)
        E.run_scene(89) return
      end,
    },
  },
}
