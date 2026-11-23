-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 95 "Онлайн" / объект 487 "Ивент"
local E = require("runtime.engine")

return {
  scene_id = 95,
  object_id = 487,
  name = "Ивент",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 20)
        E.set_pos(o, -48.500030517578, -300.67236706443)
        E.camera_off()
        while true do
          E.fb_get("Ивент", "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(85, v) end)
          if E.truthy(E.op_eq(E.gv(85), "4")) then
            E.broadcast(25)
          end
          if E.truthy(E.op_eq(E.gv(85), "3")) then
            E.broadcast(24)
          end
          if E.truthy(E.op_eq(E.gv(85), "2")) then
            E.broadcast(19)
          end
          if E.truthy(E.op_eq(E.gv(85), "0")) then
            E.broadcast(16)
          end
          if E.truthy(E.op_eq(E.gv(85), "1")) then
            E.broadcast(15)
          end
          E.wait(E.num(0.05))
          E.frame()
        end
        E.call_function(16)
        E.set_image_by_id(o, 836)
        E.call_function(15)
        E.set_image_by_id(o, 837)
        E.call_function(24)
        E.set_image_by_id(o, 947)
        E.call_function(19)
        E.set_image_by_id(o, 877)
      end,
    },
  },
}
