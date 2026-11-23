-- engine.lua — рантайм блоков Pocket Code (то, во что компилируются scripts.txt).
--
-- Все игровые данные (позиции, картинки, слои) живут в C++-движке; здесь —
-- переменные, арифметика Pocket Code, тайминги и обёртки над C-API `cb`.
-- Имена функций повторяют имена блоков: E.set_pos -> SetPositionBrick и т.д.

local E = {}

local cb = assert(_G.cb, "движок не инициализирован: нет таблицы cb")

E.cb = cb

-- планировщик подключается из boot.lua
local scheduler
function E.set_scheduler(s) scheduler = s end

---------------------------------------------------------------- переменные

local gvars = {}          -- [id] = значение
local gnames = {}         -- [id] = имя (для диагностики)
local lvars = {}          -- [handle][id] = значение
local arrays = {}         -- [id] = {значения}

function E.define_variables(list)
  for _, v in ipairs(list) do
    gnames[v.id] = v.name
    if gvars[v.id] == nil then
      gvars[v.id] = v.value or ""
    end
  end
end

function E.var_name(id)
  return gnames[id] or ("var" .. tostring(id))
end

local function num(value)
  if type(value) == "number" then return value end
  if type(value) == "boolean" then return value and 1 or 0 end
  local n = tonumber(value)
  return n or 0
end
E.num = num

local function str(value)
  if value == nil then return "" end
  if type(value) == "number" then
    if value == math.floor(value) and math.abs(value) < 1e15 then
      return string.format("%d", value)
    end
    return string.format("%.14g", value)
  end
  if type(value) == "boolean" then return value and "true" or "false" end
  return tostring(value)
end
E.str = str

function E.truthy(value)
  if type(value) == "boolean" then return value end
  if type(value) == "number" then return value ~= 0 end
  if value == nil then return false end
  local s = tostring(value)
  return s ~= "" and s ~= "false" and s ~= "0"
end

-- переменные
function E.gv(id) return gvars[id] end
function E.set_gv(id, value) gvars[id] = value end
function E.add_gv(id, delta) gvars[id] = num(gvars[id]) + num(delta) end

function E.lv(o, id)
  local t = lvars[o]
  return t and t[id]
end
function E.set_lv(o, id, value)
  lvars[o] = lvars[o] or {}
  lvars[o][id] = value
end
function E.add_lv(o, id, delta)
  lvars[o] = lvars[o] or {}
  lvars[o][id] = num(lvars[o][id]) + num(delta)
end

function E.array(id)
  arrays[id] = arrays[id] or {}
  return arrays[id]
end
function E.array_add(id, value)
  local a = E.array(id)
  a[#a + 1] = value
end
function E.array_contains(id, value)
  for _, item in ipairs(E.array(id)) do
    if str(item) == str(value) then return true end
  end
  return false
end

-- сохранение/чтение «на диск» (блоки saveVariable / readVariable)
function E.store_var(o, value, key)
  cb.store_set("var:" .. tostring(key), str(value))
end
function E.load_var(o, id)
  local stored = cb.store_get("var:" .. tostring(id))
  if stored ~= nil then
    gvars[id] = stored
  end
  return gvars[id]
end

---------------------------------------------------------------- операторы

-- Pocket Code сравнивает числа как числа, строки — как строки.
local function both_numbers(a, b)
  local na = tonumber(a)
  local nb = tonumber(b)
  if na ~= nil and nb ~= nil then
    return na, nb
  end
  return nil
end

function E.op_eq(a, b)
  local na, nb = both_numbers(a, b)
  if na then return na == nb end
  return str(a) == str(b)
end

function E.op_ne(a, b) return not E.op_eq(a, b) end

local function cmp(a, b)
  local na, nb = both_numbers(a, b)
  if na then return na, nb end
  return str(a), str(b)
end

function E.op_lt(a, b) local x, y = cmp(a, b); return x < y end
function E.op_gt(a, b) local x, y = cmp(a, b); return x > y end
function E.op_le(a, b) local x, y = cmp(a, b); return x <= y end
function E.op_ge(a, b) local x, y = cmp(a, b); return x >= y end

function E.op_add(a, b)
  local na, nb = both_numbers(a, b)
  if na then return na + nb end
  return str(a) .. str(b)                    -- «+» у Pocket Code клеит строки
end
function E.op_sub(a, b) return num(a) - num(b) end
function E.op_mul(a, b) return num(a) * num(b) end
function E.op_div(a, b)
  local d = num(b)
  if d == 0 then return 0 end
  return num(a) / d
end
function E.op_neg(a) return -num(a) end
function E.op_join(...)
  local out = {}
  for i = 1, select("#", ...) do
    out[i] = str((select(i, ...)))
  end
  return table.concat(out)
end

function E.round(v) return math.floor(num(v) + 0.5) end
function E.mod(a, b)
  local d = num(b)
  if d == 0 then return 0 end
  return num(a) % d
end
function E.log10(v) return math.log(num(v), 10) end
function E.truncate(v) return (num(v) >= 0) and math.floor(num(v)) or math.ceil(num(v)) end
function E.atan2deg(y, x) return math.deg(math.atan(y, x)) end
function E.random(a, b)
  a, b = num(a), num(b)
  if a > b then a, b = b, a end
  if a == b then return a end
  return a + math.random() * (b - a)
end

function E.str_letter(index, text)
  local i = math.floor(num(index))
  return string.sub(str(text), i, i)
end
function E.str_length(text) return #str(text) end
function E.str_contains(text, part)
  return string.find(str(text), str(part), 1, true) ~= nil
end
function E.str_sub(text, start, length)
  local i = math.floor(num(start))
  local n = math.floor(num(length))
  return string.sub(str(text), i, i + n - 1)
end

---------------------------------------------------------------- тайминги

function E.wait(seconds)
  seconds = num(seconds)
  if seconds <= 0 then seconds = 0 end
  coroutine.yield(seconds)
end

function E.frame()
  coroutine.yield(0)
end

function E.wait_until(predicate, o)
  while not E.truthy(predicate()) do
    E.frame()
  end
end

function E.time_ms()
  return cb.time() * 1000
end

function E.mark(tag)
  cb.log("script " .. tostring(tag))
end

---------------------------------------------------------------- объекты

local function obj(o)
  return o
end

function E.pos_x(o) return cb.get(o, "x") end
function E.pos_y(o) return cb.get(o, "y") end
function E.rot(o) return cb.get(o, "rotation") end
function E.size(o) return cb.get(o, "size") end
function E.alpha(o) return cb.get(o, "alpha") end
function E.bright(o) return cb.get(o, "brightness") end
function E.layer(o) return cb.get(o, "layer") end
function E.visible(o) return cb.get(o, "visible") == 1 end
function E.obj_name(o) return cb.get(o, "name") end
function E.image_number(o) return cb.get(o, "image_index") end
function E.image_name(o) return cb.get(o, "image_name") end
function E.sound_count(o) return cb.get(o, "sound_count") end
function E.body_type(o) return "dynamic" end
function E.blend(name) return name end

function E.set_pos(o, x, y)
  cb.set(o, "x", num(x))
  cb.set(o, "y", num(y))
end
function E.set_x(o, x) cb.set(o, "x", num(x)) end
function E.set_y(o, y) cb.set(o, "y", num(y)) end
function E.add_x(o, dx) cb.set(o, "x", num(E.pos_x(o)) + num(dx)) end
function E.add_y(o, dy) cb.set(o, "y", num(E.pos_y(o)) + num(dy)) end
function E.set_rot(o, r) cb.set(o, "rotation", num(r)) end
function E.add_rot(o, d) cb.set(o, "rotation", num(E.rot(o)) + num(d)) end
function E.set_size(o, s) cb.set(o, "size", num(s)) end
function E.add_size(o, d) cb.set(o, "size", num(E.size(o)) + num(d)) end
function E.set_alpha(o, a) cb.set(o, "alpha", num(a)) end
function E.add_alpha(o, d) cb.set(o, "alpha", num(E.alpha(o)) + num(d)) end
function E.set_bright(o, b) cb.set(o, "brightness", num(b)) end
function E.set_layer(o, l) cb.set(o, "layer", num(l)) end
function E.show(o) cb.set(o, "visible", 1) end
function E.hide(o) cb.set(o, "visible", 0) end
function E.go_steps(o, steps) cb.go_steps(o, num(steps)) end

function E.set_image_by_id(o, id) cb.set_image_id(o, math.floor(num(id))) end
function E.next_image(o) cb.next_image(o) end

function E.go_to(o, object_id)
  local target = cb.obj_by_id(math.floor(num(object_id)))
  if target ~= 0 then
    E.set_pos(o, E.pos_x(target), E.pos_y(target))
  end
end
function E.go_to_self(o) return o end
function E.go_to_touch(o)
  E.set_pos(o, cb.touch_x(), cb.touch_y())
end
function E.face(o, object_id)
  local target = cb.obj_by_id(math.floor(num(object_id)))
  if target == 0 then return end
  local dx = E.pos_x(target) - E.pos_x(o)
  local dy = E.pos_y(target) - E.pos_y(o)
  E.set_rot(o, math.deg(math.atan(dx, dy)))
end
function E.obj_pos_x(object_id)
  local target = cb.obj_by_id(math.floor(num(object_id)))
  return target ~= 0 and E.pos_x(target) or 0
end

-- клоны
function E.clone(object_id)
  local target = cb.obj_by_id(math.floor(num(object_id)))
  if target ~= 0 then
    local handle = cb.clone(target)
    if handle ~= 0 and clone_handler then
      clone_handler(handle, cb.get(handle, "object_id"))
    end
  end
end
function E.clone_self(o)
  local handle = cb.clone(o)
  if handle ~= 0 and clone_handler then
    clone_handler(handle, cb.get(handle, "object_id"))
  end
end
function E.delete_clone(o)
  cb.delete(o)
end

---------------------------------------------------------------- события

-- обработчики подставляет boot.lua: клоны и broadcast-сообщения
local clone_handler, broadcast_handler
function E.on_clone(fn) clone_handler = fn end
function E.on_broadcast(fn) broadcast_handler = fn end

function E.broadcast(id)
  if broadcast_handler then
    broadcast_handler(math.floor(num(id)))
  end
end

function E.call_function(id) E.broadcast(id) end
function E.broadcast_to_object(object_id, id) E.broadcast(id) end

local warned = {}
function E.warn_once(key, message)
  if not warned[key] then
    warned[key] = true
    cb.log(message)
  end
end

---------------------------------------------------------------- ввод, касания

function E.touch_x() return cb.touch_x() end
function E.touch_y() return cb.touch_y() end
function E.touch_active() return cb.touch_down() == 1 end
function E.touch_count() return cb.touch_count() end
function E.touch_id() return cb.touch_id() end
function E.touch_x_by_id(i) return cb.touch_x_by_index(math.floor(num(i))) end
function E.touch_y_by_id(i) return cb.touch_y_by_index(math.floor(num(i))) end
function E.touch_id_by_index(i) return cb.touch_id_by_index(math.floor(num(i))) end

function E.touching_object(o, object_id)
  local target = cb.obj_by_id(math.floor(num(object_id)))
  if target == 0 then return false end
  return cb.hit_o(o, target) == 1
end

---------------------------------------------------------------- сцена, звук, система

-- Переключение сцены откладывается до конца кадра: сейчас мы внутри корутины
-- скрипта, и пересоздавать планировщик «на ходу» нельзя.
local pending_scene = nil
function E.run_scene(id)
  pending_scene = math.floor(num(id))
  cb.log("runScene(" .. tostring(pending_scene) .. ") <- " ..
    string.gsub(debug.traceback("", 2), "\n", " | "))
end
function E.take_pending_scene()
  local value = pending_scene
  pending_scene = nil
  return value
end
function E.scene_name() return cb.scene_name() end
function E.stage_w() return cb.stage()[1] end
function E.stage_h() return cb.stage()[2] end

function E.play_sound(o, id) cb.play_sound(o, math.floor(num(id))) end
function E.play_sound_wait(o, id)
  cb.play_sound(o, math.floor(num(id)))
  E.wait(0.4)
end

function E.toast(text) cb.toast(str(text)) end
function E.open_link(url) cb.log("openLink: " .. str(url)) end
function E.exit_game() cb.exit() end
function E.set_bg_color(hex) cb.bg_color(str(hex)) end
function E.adaptive_off() end
function E.camera_off() end
function E.camera_focus(x, y) cb.log("focusCamera: " .. str(x) .. "," .. str(y)) end
function E.set_gravity(x, y) cb.log("gravity: " .. str(x) .. "," .. str(y)) end
function E.add_body(o, kind) cb.log("addBody(" .. str(kind) .. ")") end
function E.set_sensor(o, mode) cb.log("isSensor(" .. str(mode) .. ")") end
function E.set_hitbox(o, w, h) cb.log("hitbox " .. str(w) .. "x" .. str(h)) end
function E.noop() end
function E.unsupported(name)
  cb.unsupported(str(name))
end
function E.unknown_expr(name)
  cb.unsupported("expr:" .. str(name))
  return 0
end

-- «Firebase» эмулируется локальным хранилищем (см. engine.cpp: fb_*).
function E.fb_key(a, b, c)
  return str(a) .. "/" .. str(b) .. "/" .. str(c)
end
function E.fb_set(key, value, url)
  cb.firebase_set(str(key), str(value))
end
function E.fb_get(key, url, callback)
  local value = cb.firebase_get(str(key))
  if callback then callback(value) end
  return value
end

-- текстовые поля / показ переменных
-- Текстовые поля: EditText из Pocket Code. Если поле редактируемое и ему
-- передана переменная, подтверждённый ввод пишется в эту переменную.
E.text_vars = {}          -- [label] = id переменной
E.text_handlers = {}      -- [label] = function(value) — обработчик E.ask

function E.text_create(label, editable, w, h, varid)
  local name = str(label)
  local is_edit = str(editable) == "on"
  -- В Pocket Code EditText — тёмный текст в светлой рамке, размер подогнан
  -- под высоту поля (80 px -> примерно 35 px по высоте буквы).
  local height = num(h)
  local size = height > 0 and height * 1.5 or 40
  cb.text_create(name, 0, 0, num(w), height, "#000000", "left", is_edit and 1 or 0)
  cb.text_set(name, "value", str(E.gv(varid)))
  cb.text_set(name, "size", size)
  if is_edit and varid ~= nil then
    E.text_vars[name] = num(varid)
  end
end

-- Вызывается из boot.lua, когда пользователь закончил ввод (Enter или
-- нажатие мимо поля).
function E.on_text_commit(label, value)
  local varid = E.text_vars[label]
  if varid then
    E.set_gv(varid, value)
  end
  local handler = E.text_handlers[label]
  if handler then
    E.text_handlers[label] = nil
    E.text_vars[label] = nil
    handler(value)
  end
end

function E.text_pending(label)
  return E.text_handlers[label] ~= nil
end

-- Блок AskBrick: показывает поле ввода и ждёт ответа, не блокируя остальные
-- скрипты (ждём через E.frame()).
local ask_counter = 0
function E.ask(o, prompt, callback)
  ask_counter = ask_counter + 1
  local label = "Вопрос:" .. tostring(prompt) .. ":" .. tostring(ask_counter)
  cb.text_create(label, 0, 0, 620, 80, "#000000", "center", 1)
  cb.text_set(label, "size", 120)
  cb.text_set(label, "placeholder", str(prompt))
  cb.text_set(label, "value", "")
  E.text_handlers[label] = function(value)
    cb.text_delete(label)
    if callback then
      callback(value)
    end
  end
  cb.text_focus(label)
  while E.text_pending(label) do
    E.frame()
  end
end
function E.text_delete(label) cb.text_delete(str(label)) end
function E.text_set(label, field, a, b)
  cb.text_set(str(label), str(field), a, b)
end
E.var_texts = {}          -- ярлык -> id переменной (обновляется каждый кадр)
function E.text_show_var(varid, x, y, size, color, align)
  local label = "var:" .. tostring(varid)
  cb.text_create(label, num(x), num(y), 0, 0, str(color), str(align), 0)
  cb.text_set(label, "pos", num(x), num(y))     -- подпись «едет» за объектом
  cb.text_set(label, "value", str(E.gv(varid)))
  cb.text_set(label, "size", num(size))
  E.var_texts[label] = varid
end

-- анимации
function E.transition(o, duration, x, y)
  local steps = math.max(1, math.floor(num(duration) * 30))
  local sx, sy = num(E.pos_x(o)), num(E.pos_y(o))
  local dx = (num(x) - sx) / steps
  local dy = (num(y) - sy) / steps
  for _ = 1, steps do
    E.add_x(o, dx)
    E.add_y(o, dy)
    E.frame()
  end
  E.set_pos(o, x, y)
end

-- частицы (createRadialParticle / setPositionParticle) — заглушка
function E.particles_create(o, label, image_id)
  cb.unsupported("particles")
end
function E.particles_move(o, label, x, y)
  cb.unsupported("particles")
end

---------------------------------------------------------------- часы

function E.clock_second() return tonumber(os.date("%S")) end
function E.clock_minute() return tonumber(os.date("%M")) end
function E.clock_hour() return tonumber(os.date("%H")) end
function E.clock_day() return tonumber(os.date("%d")) end
function E.clock_month() return tonumber(os.date("%m")) end
function E.clock_year() return tonumber(os.date("%Y")) end
function E.loudness() return 0 end
function E.volume() return 100 end

return E
