-- boot.lua — загрузка проекта, активация сцен и обработка событий.
--
-- Соответствие событиям Pocket Code:
--   start          — при старте сцены
--   when_truth     — каждый кадр проверяется условие, скрипт стартует по фронту
--   touch_down    — палец коснулся объекта
--   touch_up      — палец отпущен над объектом (в Pocket Code это onTouchObject)
--   clone          — старт у клона
--   back           — кнопка «назад»

local E = require("runtime.engine")
local Scheduler = require("runtime.scheduler")

local M = {}

local cb = E.cb

local function new_scheduler()
  local scheduler = Scheduler.new()
  -- поток знает, чей он: нужно для «функций» объекта (call_function)
  scheduler:on_resume(function(handle)
    if handle and handle ~= 0 then
      E.current_object = cb.get(handle, "object_id")
    end
  end)
  E.set_scheduler(scheduler)
  return scheduler
end

local sched = new_scheduler()

local project
local layout = {}           -- стартовые позиции (assets/layout.lua)
local modules = {}          -- [scene_id] = { [object_id] = module }
local scene_index = {}      -- [scene_id] = { dir = "26", name = "Игра" }
local active_scene = 0
local truth_prev = {}       -- [ключ] = было ли условие истинным
local touch_prev = false
local back_pressed = false
local var_texts = {}        -- ярлыки текстовых полей, показывающих переменные
local emeralds = require("runtime.emeralds")
local abilities = require("runtime.abilities")

local function_scripts = {}   -- [object_id] = { [id функции] = { script =, handle = } }
local stats = { frames = 0, spawns = 0, errors = 0 }

local function key(handle, index)
  return tostring(handle) .. ":" .. tostring(index)
end

local function safe_call(fn, ...)
  local ok, err = pcall(fn, ...)
  if not ok then
    stats.errors = stats.errors + 1
    cb.log("ошибка скрипта: " .. tostring(err))
    return false
  end
  return true
end

local function spawn(script, handle, tag)
  local body = script.body
  if not body then
    return
  end
  stats.spawns = stats.spawns + 1
  sched:spawn(function()
    body(handle)
    while true do
      coroutine.yield(0)          -- тело закончилось, но поток живёт до конца кадра
    end
  end, handle, tag)
end

local function load_scene_modules(scene_id)
  if modules[scene_id] then
    return modules[scene_id]
  end
  local info = scene_index[scene_id]
  local loaded = {}
  if info then
    local ok, index = pcall(require, "scenes." .. info.dir .. ".index")
    if ok and index then
      for _, entry in ipairs(index.objects) do
        local ok2, mod = pcall(require, "scenes." .. info.dir .. "." ..
          entry.file:gsub("%.lua$", ""))
        if ok2 and mod then
          loaded[entry.id] = mod
        else
          cb.log("не загрузился модуль " .. entry.file .. ": " .. tostring(mod))
        end
      end
    else
      cb.log("нет index.lua для сцены " .. tostring(scene_id))
    end
  end
  modules[scene_id] = loaded
  return loaded
end

local function register_functions(scene_id, mods)
  function_scripts = {}
  for object_id, mod in pairs(mods or {}) do
    local handle = cb.obj_by_id(object_id)
    if handle ~= 0 then
      for _, script in ipairs(mod.scripts or {}) do
        local trigger = script.trigger
        if type(trigger) == "string" and trigger:sub(1, 9) == "function:" then
          local fid = tonumber(trigger:sub(10))
          if fid then
            function_scripts[object_id] = function_scripts[object_id] or {}
            function_scripts[object_id][fid] = { script = script, handle = handle }
          end
        end
      end
    end
  end
end

-- Активация сцены: движок уже переключён, здесь пересобираем скрипты.
local function activate(scene_id)
  active_scene = scene_id
  sched = new_scheduler()
  truth_prev = {}
  var_texts = {}
  touch_prev = false

  -- стартовые позиции объектов (см. комментарий в assets/layout.lua)
  local scene_layout = layout[scene_id] or {}
  for object_id, pos in pairs(scene_layout) do
    local handle = cb.obj_by_id(object_id)
    if handle ~= 0 then
      cb.set(handle, "x", pos.x or 0)
      cb.set(handle, "y", pos.y or 0)
    end
  end

  local mods = load_scene_modules(scene_id)
  register_functions(scene_id, mods)
  emeralds.enter_scene(scene_id)
  abilities.enter_scene(scene_id)
  for object_id, mod in pairs(mods) do
    local handle = cb.obj_by_id(object_id)
    if handle ~= 0 then
      for _, script in ipairs(mod.scripts or {}) do
        if script.trigger == "start" then
          spawn(script, handle, "start")
        end
      end
    end
  end
  -- первый проход: start-скрипты успевают выставить позиции/поля,
  -- поэтому первое же нажатие попадает туда, куда целился игрок
  sched:update(0)
  cb.log("сцена " .. tostring(scene_id) .. " " .. tostring(scene_index[scene_id] and
    scene_index[scene_id].name or "?"))
end

-- клоны: у копии запускаются скрипты с триггером «clone»
E.on_clone(function(handle, object_id)
  local mod = modules[active_scene] and modules[active_scene][object_id]
  if not mod then
    return
  end
  for _, script in ipairs(mod.scripts or {}) do
    if script.trigger == "clone" then
      spawn(script, handle, "clone")
    end
  end
end)

-- Запуск «функции N» объекта (или всех объектов, если object_id == nil).
E.on_function(function(object_id, fid)
  local started = 0
  for oid, functions in pairs(function_scripts) do
    if object_id == nil or oid == object_id then
      local entry = functions[fid]
      if entry then
        spawn(entry.script, entry.handle, "function")
        started = started + 1
      end
    end
  end
  if started == 0 then
    E.warn_once("fn" .. tostring(object_id) .. ":" .. tostring(fid),
      "функция " .. tostring(fid) .. " у объекта " .. tostring(object_id) ..
      ": нет тела в выгрузке")
  end
  return started
end)

function M.start(opts)
  opts = opts or {}
  math.randomseed(opts.seed or 20260802)
  local chunk = assert(loadfile(opts.project or "assets/project.lua"))
  project = chunk()

  local layout_path = opts.layout or ((opts.project or "assets/project.lua"):gsub("project%.lua$", "layout.lua"))
  local ok_layout, loaded_layout = pcall(loadfile, layout_path)
  if ok_layout and loaded_layout then
    layout = loaded_layout() or {}
  end

  -- Проект горизонтальный: Pocket Code отдаёт displayWidth/Height устройства,
  -- поэтому для orientation=horizontal меняем местами стороны сцены.
  local stage_w, stage_h = project.stage.w, project.stage.h
  if (project.orientation or "horizontal") == "horizontal" and stage_h > stage_w then
    stage_w, stage_h = stage_h, stage_w
  end
  cb.stage(stage_w, stage_h)
  E.define_variables(project.variables or {})
  -- профиль (значения переменных до старта): нужен, чтобы пропустить регистрацию
  if opts.profile then
    for id, value in pairs(opts.profile) do
      E.set_gv(tonumber(id), value)
    end
  end
  for _, scene in ipairs(project.scenes) do
    scene_index[scene.id] = { dir = scene.slug:sub(1, 2), name = scene.name }
    cb.scene_define(scene.id, scene.name)
    for _, object in ipairs(scene.objects) do
      cb.object_define(scene.id, object.id, object.name, 0, 0, 0,
        tonumber(object.size) or 1, object.images, object.sounds)
    end
  end
  cb.finalize()

  local start_scene = opts.scene
  if not start_scene then
    start_scene = project.scenes[1].id
  end
  if not cb.scene_run(start_scene) then
    error("не удалось открыть сцену " .. tostring(start_scene))
  end
  activate(start_scene)
  return true
end

-- Нажатие достаётся самому верхнему спрайту, у которого есть непрозрачный
-- пиксель в этой точке (прозрачные подложки больше не «съедают» кнопки).
-- Если у него нет ни одного скрипта на нажатие, ищем ниже — так работают
-- кнопки, прикрытые декоративным слоем.
local function fire_touch_triggers(x, y)
  local handles = cb.sprites_at(x, y)
  for _, handle in ipairs(handles) do
    local object_id = cb.get(handle, "object_id")
    local mod = modules[active_scene] and modules[active_scene][object_id]
    if mod then
      local fired = 0
      local will_release = false
      for _, script in ipairs(mod.scripts or {}) do
        if script.trigger == "tap" or script.trigger == "touch_down" then
          spawn(script, handle, script.trigger)
          fired = fired + 1
        elseif script.trigger == "touch_up" then
          will_release = true
        end
      end
      if fired > 0 or will_release then
        -- звук нажатия: настоящий MP3 проекта (у кнопки свой или общий
        -- «Нажатие»). В игре и на загрузке не щёлкаем — там управление.
        if active_scene ~= 26 and active_scene ~= 42 then
          cb.click_sound(handle)
        end
        return fired
      end
    end
  end
  return 0
end

-- Отпускание пальца: срабатывают скрипты «onTouchObject» (touch_up) того
-- объекта, над которым палец отпустили. Именно так в оригинале устроены
-- кнопки меню (подсветка на touch_down, переход на отпускании) и атака в игре
-- (прицел держится, пока палец на кнопке, урон — при отпускании).
local function fire_release_triggers(handles)
  for _, handle in ipairs(handles) do
    local object_id = cb.get(handle, "object_id")
    local mod = modules[active_scene] and modules[active_scene][object_id]
    if mod then
      local fired = 0
      for _, script in ipairs(mod.scripts or {}) do
        if script.trigger == "touch_up" then
          spawn(script, handle, script.trigger)
          fired = fired + 1
        end
      end
      if fired > 0 then
        return fired
      end
    end
  end
  return 0
end

-- Нажатие: сначала текстовые поля (они всегда поверх спрайтов),
-- потом скрипты объектов.
local function handle_press(x, y)
  if cb.text_focus_at(x, y) == 1 then
    return
  end
  cb.text_blur()
  fire_touch_triggers(x, y)
end

local function poll_presses()
  while true do
    local x, y = cb.poll_press()
    if x == nil then
      break
    end
    handle_press(x, y)
  end
end

-- Подтверждённый ввод из текстового поля: пишем в переменную и/или
-- вызываем обработчик E.ask.
local function poll_releases()
  while true do
    local x, y, handles = cb.poll_release()
    if x == nil then
      break
    end
    fire_release_triggers(handles)
  end
end

local function poll_text_commits()
  while true do
    local label, value = cb.poll_text_commit()
    if label == nil then
      break
    end
    E.on_text_commit(label, value)
  end
end
local function fire_back()
  back_pressed = true
end

local function poll_truths()
  local mods = modules[active_scene] or {}
  for object_id, mod in pairs(mods) do
    local handle = cb.obj_by_id(object_id)
    if handle ~= 0 then
      for index, script in ipairs(mod.scripts or {}) do
        if script.trigger == "when_truth" and script.cond then
          local k = key(handle, index)
          local now = false
          local ok, result = pcall(script.cond, handle)
          if ok then
            now = E.truthy(result)
          else
            stats.errors = stats.errors + 1
            cb.log("ошибка условия: " .. tostring(result))
          end
          if now and not truth_prev[k] then
            truth_prev[k] = true
            spawn(script, handle, "when_truth")
          elseif not now then
            truth_prev[k] = false
          end
        end
      end
    end
  end
end

local function poll_back()
  local mods = modules[active_scene] or {}
  if back_pressed then
    back_pressed = false
    for object_id, mod in pairs(mods) do
      local handle = cb.obj_by_id(object_id)
      if handle ~= 0 then
        for _, script in ipairs(mod.scripts or {}) do
          if script.trigger == "back" then
            spawn(script, handle, "back")
          end
        end
      end
    end
  end
end

local function update_var_texts()
  for label, varid in pairs(E.var_texts) do
    cb.text_set(label, "value", E.str(E.gv(varid)))
  end
  for label, varid in pairs(var_texts) do
    cb.text_set(label, "value", E.str(E.gv(varid)))
  end
end

-- Для тестов и отладки: выставить глобальную переменную из CLI (--set-var).
function M.debug_set_var(id, value)
  E.set_gv(tonumber(id), value)
end

-- Сцена «Больше» (31): поле промокода — белая плашка без подсказки-слова
-- («Промокод» не пишем), текст внутри тёмный, чтобы читался на белом.
-- Поле создаёт скрипт объекта «Ввести», поэтому правим его каждый кадр:
-- text_set по несуществующему полю молча ничего не делает.
local function style_promo_field(scene_id)
  if scene_id ~= 31 then
    return
  end
  E.cb.text_set("Промокод", "visible", 1)
  E.cb.text_set("Промокод", "placeholder", "")
  E.cb.text_set("Промокод", "align", "center")
  E.cb.text_set("Промокод", "size", 56)
  E.cb.text_set("Промокод", "color", "#1b3c5a")
end

function M.frame(dt)
  stats.frames = stats.frames + 1

  poll_truths()
  poll_back()
  sched:update(dt)
  update_var_texts()

  poll_presses()
  poll_releases()
  poll_text_commits()
  emeralds.check_frame(active_scene)
  abilities.check_frame(active_scene)
  style_promo_field(active_scene)

  local pending = E.take_pending_scene and E.take_pending_scene()
  if pending and pending ~= active_scene then
    if cb.scene_run(pending) then
      activate(pending)
    end
  end
end

-- Диагностика кнопок: нажатие в центр объекта должно доходить до него самого,
-- а не до декоративного слоя сверху.
local function has_tap_scripts(scene_modules, object_id)
  local mod = scene_modules and scene_modules[object_id]
  if not mod then
    return false
  end
  for _, script in ipairs(mod.scripts or {}) do
    if script.trigger == "tap" or script.trigger == "touch_down" or
        script.trigger == "touch_up" then
      return true
    end
  end
  return false
end

function M.check_buttons()
  local mods = modules[active_scene] or {}
  local report = {}
  for object_id, mod in pairs(mods) do
    local handle = cb.obj_by_id(object_id)
    if handle ~= 0 and cb.get(handle, "visible") == 1 and has_tap_scripts(mods, object_id) then
      local x = cb.get(handle, "x")
      local y = cb.get(handle, "y")
      -- Кнопка может быть перекрыта другой кнопкой честно (например, полоска
      -- поверх большой панели), поэтому ищем по площади рамки: есть ли хоть
      -- одна точка, где нажатие достаётся именно ей.
      local w = cb.get(handle, "width")
      local h = cb.get(handle, "height")
      if w <= 0 or h <= 0 then
        -- объект без картинки: в Pocket Code он тоже невидим, нажимать некуда
        report[#report + 1] = {
          name = mod.name,
          object_id = object_id,
          x = x,
          y = y,
          reached = true,
          invisible = true,
          hits = 0,
          samples = 0,
          blocker = "нет картинки",
        }
        goto continue
      end
      local samples = 0
      local hits = 0
      local blocker = nil
      local steps = 7
      for ix = 0, steps - 1 do
        for iy = 0, steps - 1 do
          local px = x - w / 2 + (w * (ix + 0.5) / steps)
          local py = y - h / 2 + (h * (iy + 0.5) / steps)
          samples = samples + 1
          for _, other in ipairs(cb.sprites_at(px, py)) do
            if other == handle then
              hits = hits + 1
              break
            end
            if has_tap_scripts(mods, cb.get(other, "object_id")) then
              blocker = blocker or cb.get(other, "name")
              break
            end
          end
        end
      end
      report[#report + 1] = {
        name = mod.name,
        object_id = object_id,
        x = x,
        y = y,
        reached = hits > 0,
        hits = hits,
        samples = samples,
        blocker = blocker,
      }
      ::continue::
    end
  end
  return { scene = active_scene, buttons = report }
end

-- API для тестов/фронтенда
function M.stats()
  return {
    frames = stats.frames,
    spawns = stats.spawns,
    errors = stats.errors,
    threads = sched:count(),
    scene = active_scene,
  }
end

function M.press(x, y)
  cb.press(x, y)
end

function M.release()
  cb.release()
end

function M.back()
  fire_back()
end

function M.run_scene(id)
  if cb.scene_run(id) then
    activate(id)
  end
end

return M
