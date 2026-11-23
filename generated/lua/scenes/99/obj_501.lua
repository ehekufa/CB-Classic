-- АВТОГЕНЕРАЦИЯ: tools/catrobat2lua.py — не редактировать вручную.
-- Сцена 99 "Онлайн (1)" / объект 501 "Ивент"
local E = require("runtime.engine")

return {
  scene_id = 99,
  object_id = 501,
  name = "Ивент",
  scripts = {
    { -- скрипт 1: start
      trigger = "start",
      body = function(o)
        E.set_size(o, 20)
        E.set_pos(o, -48.500030517578, -300.67236706443)
        E.camera_off()
        E.call_function(25)
        E.set_image_by_id(o, 1067)
        E.call_function(16)
        E.set_image_by_id(o, 856)
        E.call_function(20)
        E.set_image_by_id(o, 895)
        E.call_function(19)
        E.set_image_by_id(o, 881)
        E.call_function(15)
        E.set_image_by_id(o, 857)
      end,
    },
  },
}
