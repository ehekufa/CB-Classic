-- valentines.lua — счётчик валентинок поверх интерфейса.
--
-- В оригинале количество валентинок жило в Firebase (ключ «Имя/Пароль/В»), а в
-- меню его показывала надпись var:92 у «Конфеты» (сцена 1). Без сети значение
-- приходило пустым, поэтому счётчика не было видно вообще. Здесь валентинки
-- лежат в локальном хранилище порта и показываются одной надписью, которая
-- рисуется поверх всех спрайтов сцены:
--   * ключ хранилища — «var:93»: тот же, что у блоков saveVariable/readVariable
--     оригинала (сцена «Игра», промокоды, онлайн-победы пишут туда as is);
--   * переменные 93 «Валентинка» и 92 «Валентинка (визуал)» держатся равными
--     счёту — их проверяют цена класса «Влюбивик» (500) и античит (100000);
--   * значение дублируется в локальную «Firebase»-запись, чтобы исходный скрипт
--     меню (fbGetValue) получал не пустоту, а число;
--   * сама надпись — текстовое поле «hud:валентинки»: оно создаётся каждый кадр
--     и поэтому видно в любом месте экрана и поверх любой картинки.
local E = require("runtime.engine")

local M = {}

local KEY = "var:93"          -- локальное хранилище: счёт валентинок
local VAR_COUNT = 93          -- «Валентинка»
local VAR_VISUAL = 92         -- «Валентинка (визуал)»
local LABEL = "hud:валентинки"
local OLD_LABEL = "var:92"    -- надпись оригинала у «Конфеты» (прячем, чтобы не двоилось)
local COLOR = "#e01b45"
local SIZE = 120
local TOP_MARGIN = 54
-- до регистрации и на загрузке счётчик не нужен
local HIDDEN_SCENES = { [42] = true, [77] = true }

local count = 0

local function fmt(n)
  local text = tostring(math.floor(n))
  local out, digits = "", 0
  for i = #text, 1, -1 do
    out = text:sub(i, i) .. out
    digits = digits + 1
    if digits % 3 == 0 and i > 1 then
      out = " " .. out
    end
  end
  return out
end

local function store(value)
  count = math.max(0, math.floor(value))
  E.cb.store_set(KEY, tostring(count))
  E.set_gv(VAR_COUNT, count)
  E.set_gv(VAR_VISUAL, count)
  -- исходный скрипт меню читает «Firebase»-ключ вида Имя/Пароль/В
  E.cb.firebase_set(E.fb_key(E.gv(61), E.gv(62), "В"), tostring(count))
end

function M.count()
  return count
end

function M.set(value, silent)
  store(value)
  if not silent then
    E.toast("Валентинки: " .. fmt(count))
  end
  return count
end

function M.add(delta, silent)
  return M.set(count + (delta or 1), silent)
end

-- Счётчик поверх интерфейса: белая цифра с подписью на красной плашке.
local function draw_hud(scene_id)
  if HIDDEN_SCENES[scene_id] then
    return
  end
  local _, stage_h = E.cb.stage()
  local x = 0
  local y = stage_h / 2 - TOP_MARGIN
  E.cb.text_create(LABEL, x, y, 0, 0, COLOR, "center", 0)
  E.cb.text_set(LABEL, "pos", x, y)
  E.cb.text_set(LABEL, "size", SIZE)
  E.cb.text_set(LABEL, "color", COLOR)
  E.cb.text_set(LABEL, "align", "center")
  E.cb.text_set(LABEL, "value", "Валентинки: " .. fmt(count))
  E.cb.text_set(LABEL, "visible", 1)
  -- надпись оригинала у «Конфеты» гасим: счётчик теперь один, поверх всего
  E.cb.text_set(OLD_LABEL, "visible", 0)
end

function M.enter_scene(scene_id)
  local stored = tonumber(E.cb.store_get(KEY)) or 0
  store(stored)
  draw_hud(scene_id)
end

function M.check_frame(scene_id)
  -- собрали «Леденец» в игре, ввели промокод или победили в онлайне:
  -- переменную изменил скрипт оригинала — забираем значение в счётчик
  local live = tonumber(E.gv(VAR_COUNT))
  if live and live ~= count then
    store(live)
  elseif tonumber(E.gv(VAR_VISUAL)) ~= count then
    E.set_gv(VAR_VISUAL, count)
  end
  draw_hud(scene_id)
end

function M.reset()                 -- для тестов
  store(0)
end

return M
