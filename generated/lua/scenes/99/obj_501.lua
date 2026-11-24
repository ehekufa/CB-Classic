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
        E.camera_off(o)
      end,
    },
    { -- скрипт 2: function:25
      trigger = "function:25",
      body = function(o)
        E.set_image_by_id(o, 1067)
      end,
    },
    { -- скрипт 3: function:16
      trigger = "function:16",
      body = function(o)
        E.set_image_by_id(o, 856)
      end,
    },
    { -- скрипт 4: function:20
      trigger = "function:20",
      body = function(o)
        E.set_image_by_id(o, 895)
      end,
    },
    { -- скрипт 5: function:19
      trigger = "function:19",
      body = function(o)
        E.set_image_by_id(o, 881)
      end,
    },
    { -- скрипт 6: function:15
      trigger = "function:15",
      body = function(o)
        E.set_image_by_id(o, 857)
      end,
    },
  },
}
