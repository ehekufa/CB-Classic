-- scheduler.lua — параллельные «скрипты» Pocket Code на корутинах Lua.
--
-- Каждый скрипт объекта живёт в своей корутине. `E.wait(0.5)` и `E.frame()`
-- приостанавливают её, а планировщик возобновляет по времени/на следующем кадре.
-- Это точная модель поведения Pocket Code, где все скрипты выполняются параллельно.

local Scheduler = {}
Scheduler.__index = Scheduler

function Scheduler.new()
  return setmetatable({ threads = {}, clock = 0, alive = 0 }, Scheduler)
end

-- Добавляет поток. fn получает объект-владелец (handle).
function Scheduler:spawn(fn, handle, tag)
  local co = coroutine.create(fn)
  local thread = {
    co = co,
    handle = handle,
    wait = 0,
    dead = false,
    tag = tag,
  }
  self.threads[#self.threads + 1] = thread
  self.alive = self.alive + 1
  return thread
end

function Scheduler:on_resume(fn)
  self.hook = fn
end

local function resume(thread)
  -- аргументы в корутину не передаём: объект-владелец захвачен замыканием
  local ok, delay = coroutine.resume(thread.co)
  if not ok then
    io.stderr:write("[lua] ошибка в скрипте: " .. tostring(delay) .. "\n")
    thread.dead = true
    return
  end
  if coroutine.status(thread.co) == "dead" then
    thread.dead = true
    return
  end
  if type(delay) ~= "number" then
    delay = 0
  end
  thread.wait = delay
end

function Scheduler:update(dt)
  self.clock = self.clock + dt
  local survivors = {}
  local list = self.threads
  self.threads = survivors
  for i = 1, #list do
    local thread = list[i]
    if thread.dead then
      self.alive = self.alive - 1
    else
      if thread.wait > 0 then
        thread.wait = thread.wait - dt
      end
      if thread.wait <= 0 then
        if self.hook then self.hook(thread.handle) end
        resume(thread)
      end
      if thread.dead then
        self.alive = self.alive - 1
      else
        survivors[#survivors + 1] = thread
      end
    end
  end
end

function Scheduler:count()
  return #self.threads
end

function Scheduler:kill_for(handle)
  for i = 1, #self.threads do
    if self.threads[i].handle == handle then
      self.threads[i].dead = true
    end
  end
end

return Scheduler
