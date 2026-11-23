-- abilities.lua — перезарядка способности класса (кнопка «Супер атака»).
--
-- В оригинале у каждого класса своя пауза: у пули 5.5 с, у лазера 2 с, у
-- «влюбивика» 2.1 с, у снаряда 0.18 с, у рывка Азума вообще нет. Получалось
-- неровно: одни способности фактически без перезарядки, другие заметно дольше.
-- По просьбе «перезарядка у всех примерно 15 секунд максимум» порт ставит
-- единое правило: после применения кнопка гаснет (яркость 85, как в оригинале)
-- и ровно COOLDOWN секунд не принимает нажатий, потом снова становится яркой.
--
-- Скрипт кнопки сам проверяет «яркость == 100», поэтому серый цвет — это и есть
-- блокировка: во время перезарядки кнопка не срабатывает, даже если жать её
-- часто. Всё, что делает модуль — держит кнопку серой, пока идёт пауза.
local E = require("runtime.engine")

local M = {}

local SCENE = 26           -- «Игра»
local BUTTON = 26          -- объект «Супер атака» — способность класса
local COOLDOWN = 15.0      -- секунд, максимум по просьбе
local DIM = 85             -- яркость «на перезарядке»

local ready_at = 0.0       -- когда снова можно
local cooling = false

local function button()
  local handle = E.cb.obj_by_id(BUTTON)
  if handle == 0 then
    return nil
  end
  return handle
end

function M.enter_scene(scene_id)
  cooling = false
  ready_at = 0.0
  if scene_id ~= SCENE then
    return
  end
  local o = button()
  if o then
    E.set_bright(o, 100)
  end
end

function M.check_frame(scene_id)
  if scene_id ~= SCENE then
    return
  end
  local o = button()
  if not o then
    return
  end
  local now = E.cb.time()
  local bright = E.bright(o)
  if not cooling then
    -- нажатие видно по тому, что скрипт кнопки погасил её (яркость 85)
    if bright < 99.5 then
      cooling = true
      ready_at = now + COOLDOWN
    end
    return
  end
  if now >= ready_at then
    cooling = false
    E.set_bright(o, 100)
  elseif bright > DIM then
    -- скрипт кнопки по окончании анимации вернул 100 — снова гасим,
    -- пока перезарядка не закончилась
    E.set_bright(o, DIM)
  end
end

function M.remaining(scene_id)
  if scene_id ~= SCENE or not cooling then
    return 0
  end
  return math.max(0, ready_at - E.cb.time())
end

function M.cooldown()
  return COOLDOWN
end

function M.reset()                 -- для тестов
  cooling = false
  ready_at = 0.0
end

return M
