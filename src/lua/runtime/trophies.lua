-- trophies.lua — кубки за победу.
--
-- В оригинале счёт кубков (переменная 24, «К») уходит в Firebase и оттуда же
-- читается; без сети он не сохранялся и нигде не показывался, поэтому победа
-- «ничего не давала». Здесь кубки лежат в локальном хранилище порта:
--   * победа над врагом в сцене «Игра» = +5 кубков (как в исходных скриптах);
--   * счёт сразу пишется в переменную 24 (её проверяют достижения);
--   * в меню и в достижениях показывается строка «Кубки: N».
local E = require("runtime.engine")

local M = {}

local KEY = "cups"          -- ключ локального хранилища
local PER_WIN = 5           -- столько даёт одна победа (как add_gv(24, 5))
local HUD_LABEL = "кубки-счёт"
local HUD_SCENES = { [1] = true, [96] = true, [31] = true, [89] = true }

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

function M.set(value, silent)
  local total = math.max(0, math.floor(value))
  E.cb.store_set(KEY, tostring(total))
  E.set_gv(24, total)                     -- переменная «К» из оригинала
  M.update_hud()
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

function M.update_hud()
  local text = "Кубки: " .. fmt(M.cups())
  -- поле пересоздаётся после каждой смены сцены (тексты сцены живут отдельно)
  E.cb.text_create(HUD_LABEL, -590, -320, 340, 60, "#2b2b2b", "left", 0)
  E.cb.text_set(HUD_LABEL, "size", 44)
  E.cb.text_set(HUD_LABEL, "value", text)
end

function M.hud_visible(flag)
  E.cb.text_set(HUD_LABEL, "visible", flag and 1 or 0)
end

-- Вызывается при входе в сцену: подтянуть локальный счёт и показать его там,
-- где ему место (меню/достижения/магазин), не трогая кнопки.
function M.enter_scene(scene_id)
  win_awarded = false
  baseline = M.cups()
  low_hp_frames = 0
  E.set_gv(24, baseline)
  if HUD_SCENES[scene_id] then
    M.update_hud()
    M.hud_visible(true)
  else
    M.hud_visible(false)
  end
end

-- Проверка победы: в сцене «Игра» здоровье врага (переменная 4) упало до нуля.
-- Кубки начисляет исходный скрипт (add_gv(24, 5)); мы не начисляем второй раз,
-- а сохраняем его прибавку и показываем игроку, сколько стало.
function M.check_frame(scene_id)
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
end

return M
