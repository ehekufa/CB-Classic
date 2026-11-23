-- trophies.lua — кубки за победу.
--
-- В оригинале счёт кубков (переменная 24, «К») уходит в Firebase и оттуда же
-- читается; без сети он не сохранялся и нигде не показывался, поэтому победа
-- «ничего не давала». Здесь кубки лежат в локальном хранилище порта:
--   * победа над врагом в сцене «Игра» = +5 кубков (как в исходных скриптах);
--   * счёт пишется в переменную 24 (её проверяют достижения) и в переменную
--     91 «Кубки (визуал)»;
--   * в меню число стоит прямо в жёлтой полосе панели «Путь к богатству»
--     (поле var:91 самой сцены), белым и без иконки кубка — иконку из
--     картинки панели убрал tools/extract_assets.py, отдельного HUD в углу нет.
local E = require("runtime.engine")

local M = {}

local KEY = "cups"          -- ключ локального хранилища
local PER_WIN = 5           -- столько даёт одна победа (как add_gv(24, 5))

-- Панель «Путь к богатству»: спрайт 374x204 в точке (495, 180), жёлтая полоса
-- занимает нижние 70 px. Счёт рисует текстовое поле сцены (var:91), которое
-- движок обновляет из переменной 91 каждый кадр.
local PANEL_SCENE = 1
local PANEL_LABEL = "var:91"
local PANEL_X = 495
local PANEL_Y = 115
local PANEL_SIZE = 140
local PANEL_COLOR = "#ffffff"

local win_awarded = false   -- чтобы одна победа не засчиталась дважды
local baseline = 0          -- счёт кубков на входе в сцену (переменная 24)
local low_hp_frames = 0     -- сколько кадров подряд враг «мёртв»

local function fmt(n)
  -- 12500 -> «12 500»
  local text = tostring(math.floor(n))
  local out, count = "", 0
  for i = #text, 1, -1 do
    out = text:sub(i, i) .. out
    count = count + 1
    if count % 3 == 0 and i > 1 then
      out = " " .. out
    end
  end
  return out
end

function M.cups()
  local stored = E.cb.store_get(KEY)
  local value = tonumber(stored)
  if not value then
    value = 0
  end
  return value
end

function M.toast(text, seconds)
  E.toast(text)
end

-- Счёт в жёлтой полосе панели: белым, по центру полосы, без иконки.
-- Поле пересоздают стартовые скрипты сцены, поэтому стиль выставляем каждый
-- кадр — это пять дешёвых вызовов, зато оформление не «слетает».
function M.show_panel()
  E.cb.text_set(PANEL_LABEL, "color", PANEL_COLOR)
  E.cb.text_set(PANEL_LABEL, "size", PANEL_SIZE)
  E.cb.text_set(PANEL_LABEL, "pos", PANEL_X, PANEL_Y)
  E.cb.text_set(PANEL_LABEL, "align", "center")
  E.cb.text_set(PANEL_LABEL, "visible", 1)
end

function M.set(value, silent)
  local total = math.max(0, math.floor(value))
  E.cb.store_set(KEY, tostring(total))
  E.set_gv(24, total)                     -- переменная «К» из оригинала
  E.set_gv(91, total)                     -- «Кубки (визуал)» — панель в меню
  if not silent then
    M.toast("Кубки: " .. fmt(total))
  end
  return total
end

function M.add(delta, silent)
  local total = M.set(M.cups() + (delta or PER_WIN), true)
  if not silent then
    M.toast("Победа! +" .. fmt(delta or PER_WIN) .. " кубков, всего " .. fmt(total))
  end
  return total
end

-- Вызывается при входе в сцену: подтянуть локальный счёт в переменные.
function M.enter_scene(scene_id)
  win_awarded = false
  baseline = M.cups()
  low_hp_frames = 0
  E.set_gv(24, baseline)
  E.set_gv(91, baseline)
  if scene_id == PANEL_SCENE then
    M.show_panel()
  end
end

-- Проверка победы: в сцене «Игра» здоровье врага (переменная 4) упало до нуля.
-- Кубки начисляет исходный скрипт (add_gv(24, 5)); мы не начисляем второй раз,
-- а сохраняем его прибавку и показываем игроку, сколько стало.
function M.check_frame(scene_id)
  if scene_id == PANEL_SCENE then
    -- сцена рисует значение переменной 91 сама: держим его равным счёту
    if tonumber(E.gv(91)) ~= M.cups() then
      E.set_gv(91, M.cups())
    end
    M.show_panel()
  end
  if scene_id ~= 26 or win_awarded then
    return
  end
  local hp = tonumber(E.gv(4))
  local current = tonumber(E.gv(24)) or baseline
  if hp and hp < 1 then
    low_hp_frames = low_hp_frames + 1
  else
    low_hp_frames = 0
    return
  end
  if current > baseline then
    win_awarded = true
    M.set(current, true)                        -- в локальное хранилище
    M.toast("Победа! +" .. fmt(current - baseline) .. " кубков, всего " .. fmt(current))
    return
  end
  if low_hp_frames >= 3 then
    -- скрипт почему-то не начислил (например, переменная 24 сброшена) — выдаём сами
    win_awarded = true
    M.add(PER_WIN)
  end
end

function M.reset()                 -- для тестов
  E.cb.store_set(KEY, "0")
  E.set_gv(24, 0)
  E.set_gv(91, 0)
end

return M
