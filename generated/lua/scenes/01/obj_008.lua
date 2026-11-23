-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 1 "Меню" / объект 8 "Путь к богатству"
local E = require("runtime.engine")

return {
  scene_id = 1,
  object_id = 8,
  name = "Путь к богатству",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_pos(o, 495, 180)
        while true do
          E.fb_get(E.fb_key(E.gv(61), E.gv(62), "К"), "https://cubic-battle-default-rtdb.firebaseio.com/", function(v) E.set_gv(91, v) end)
          E.text_show_var(91, 365, 120, 180, "#FFFFFF", "left")
          if E.truthy(E.op_ge(E.gv(24), 1000000)) then
            E.set_gv(108, "ban")
            E.store_var(o, E.gv(108))
          else
            E.set_gv(108, "unban")
            E.store_var(o, E.gv(108))
          end
          E.wait(E.num(0.5))
          E.frame()
        end
      end,
    },
  },
}
