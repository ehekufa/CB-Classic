-- boot.lua — загрузка проекта, активация сцен и обработка событий.
--
-- Соответствие событиям Pocket Code:
--   start          — при старте сцены
--   when_truth     — каждый кадр проверяется условие, скрипт стартует по фронту
--   tap/touch_down — нажатие на объект
--   clone          — старт у клона
--   back           — кнопка «назад»

local E = require("runtime.engine")
local Scheduler = require("runtime.scheduler")

local M = {}

local cb = E.cb
local sched = Scheduler.new()
E.set_scheduler(sched)

local project
local layout = {}           -- стартовые позиции (assets/layout.lua)
local modules = {}          -- [scene_id] = { [object_id] = module }
local scene_index = {}      -- [scene_id] = { dir = "26", name = "Игра" }
local active_scene = 0
local truth_prev = {}       -- [ключ] = было ли условие истинным
local touch_prev = false
local back_pressed = false
local var_texts = {}        -- ярлыки текстовых полей, показывающих переменные
local broadcast_receivers = {}
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

local function register_broadcasts(scene_id)
  broadcast_receivers = {}
  -- в исходном проекте получателей broadcast-сообщений нет (вызовы «функций»
  -- остались от модифицированной сборки Pocket Code), поэтому реестр пуст
end

-- Активация сцены: движок уже переключён, здесь пересобираем скрипты.
local function activate(scene_id)
  active_scene = scene_id
  sched = Scheduler.new()
  E.set_scheduler(sched)
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
  register_broadcasts(scene_id)
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

E.on_broadcast(function(id)
  local receivers = broadcast_receivers[id]
  if not receivers or #receivers == 0 then
    E.warn_once("bc" .. tostring(id),
      "broadcast " .. tostring(id) .. ": получателей в проекте нет (особенность сборки)")
    return
  end
  for _, receiver in ipairs(receivers) do
    spawn(receiver.script, receiver.handle, "broadcast")
  end
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

local function topmost_at(x, y)
  local best_handle, best_layer = 0, -1e9
  local handles = cb.handles()
  for _, handle in ipairs(handles) do
    if cb.get(handle, "visible") == 1 and cb.hit(handle, x, y) == 1 then
      local layer = cb.get(handle, "layer")
      if layer >= best_layer then
        best_layer = layer
        best_handle = handle
      end
    end
  end
  return best_handle
end

local function fire_touch_triggers(x, y)
  local handle = topmost_at(x, y)
  if handle == 0 then
    return
  end
  local object_id = cb.get(handle, "object_id")
  local mod = modules[active_scene] and modules[active_scene][object_id]
  if not mod then
    return
  end
  for _, script in ipairs(mod.scripts or {}) do
    if script.trigger == "tap" or script.trigger == "touch_down" then
      spawn(script, handle, script.trigger)
    end
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

function M.frame(dt)
  stats.frames = stats.frames + 1

  local down = cb.touch_down() == 1
  if down and not touch_prev then
    fire_touch_triggers(cb.touch_x(), cb.touch_y())
  end
  touch_prev = down

  poll_truths()
  poll_back()
  sched:update(dt)
  update_var_texts()

  local pending = E.take_pending_scene and E.take_pending_scene()
  if pending and pending ~= active_scene then
    if cb.scene_run(pending) then
      activate(pending)
    end
  end
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
