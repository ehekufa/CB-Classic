-- emeralds.lua — счётчик изумрудов поверх интерфейса.
--
-- В прошлой версии игры валютой были «валентинки» (переменная 93), а в новой
-- версии это же место заняли «Изумруды»: переменную 93 начисляют подбор кристалла
-- в игре (сцена 26), магазин яиц (сцены 89/104/105) и промокоды, а сохраняет и
-- читает сама игра блоками saveVariable/readVariable. Поэтому модуль ничего не
-- пишет — он только показывает число:
--   * в меню число рисует сама сцена (поле var:93 у зелёного изумруда) — оно
--     белое и без подписи, второй счётчик там не нужен;
--   * в остальных сценах — по центру сверху, чтобы не закрывать кнопки атаки;
--     там поле сцены гасим, чтобы число не двоилось.
--
-- ВАЖНО: переменные 91 и 92 в новой версии — это ROT2/ROT3 (углы поворота
-- кубов в онлайне), трогать их нельзя.
local E = require("runtime.engine")

local M = {}

local VAR = 93                 -- «Изумруды»
local LABEL = "hud:изумруды"
local SHADOW = "hud:00тень"    -- рисуется раньше счётчика (std::map по имени)
local PROJECT_LABEL = "var:93" -- надпись сцены у объекта «Изумруды» (прячем)
local COLOR = "#ffffff"
local SHADOW_COLOR = "#1b3c5a"
local SIZE = 110
local TOP_MARGIN = 20
local MENU_SCENE = 1
local HIDDEN_SCENES = { [42] = true }

local value = 0

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

local function draw_number(name, x, y, color)
  E.cb.text_create(name, x, y, 0, 0, color, "center", 0)
  E.cb.text_set(name, "pos", x, y)
  E.cb.text_set(name, "size", SIZE)
  E.cb.text_set(name, "color", color)
  E.cb.text_set(name, "align", "center")
  E.cb.text_set(name, "value", fmt(value))
  E.cb.text_set(name, "visible", 1)
end

local function draw_hud(scene_id)
  if HIDDEN_SCENES[scene_id] then
    return
  end
  local _, stage_h = E.cb.stage()
  local x, y = 0, stage_h / 2 - TOP_MARGIN - SIZE / 2
  if scene_id == MENU_SCENE then
    -- в меню показывает сама сцена: своё поле белое, без подписи и стоит у иконки
    E.cb.text_set(PROJECT_LABEL, "visible", 1)
    E.cb.text_set(LABEL, "visible", 0)
    E.cb.text_set(SHADOW, "visible", 0)
    return
  end
  -- Вне меню счётчика изумрудов не видно вовсе: сколько их, видно только в
  -- меню (в «Больше», «Игре» и остальных сценах числа быть не должно).
  E.cb.text_set(LABEL, "visible", 0)
  E.cb.text_set(SHADOW, "visible", 0)
  E.cb.text_set(PROJECT_LABEL, "visible", 0)
end

function M.count()
  return value
end

function M.enter_scene(scene_id)
  value = tonumber(E.gv(VAR)) or 0
  draw_hud(scene_id)
end

function M.check_frame(scene_id)
  -- игра начислила или потратила изумруды — подхватываем значение
  value = tonumber(E.gv(VAR)) or 0
  draw_hud(scene_id)
end

return M
