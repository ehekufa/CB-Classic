# Сборка порта Cubic battle (C + C++ + Lua) без внешних зависимостей.
#
#   make            — headless-сборка (по умолчанию)
#   make run        — прогон 90 кадров сцены «Меню» в out/frames
#   make test       — самотесты (PNG, конвертер, движок)
#   make SDL2=1     — сборка с окном (нужен libsdl2-dev)
#   make clean

CC       ?= cc
CXX      ?= c++
CFLAGS   ?= -O2 -std=c11 -Wall -Wextra -MMD -MP
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -MMD -MP
LDFLAGS  ?=
LDLIBS   ?= -lm

BUILD    := build
BIN      := $(BUILD)/cb-classic

LUA_SRC  := $(wildcard third_party/lua-5.4/*.c)
C_SRC    := $(wildcard src/c/*.c)
CPP_SRC  := $(wildcard src/cpp/*.cpp)

LUA_OBJ  := $(patsubst third_party/lua-5.4/%.c,$(BUILD)/lua/%.o,$(LUA_SRC))
C_OBJ    := $(patsubst src/c/%.c,$(BUILD)/c/%.o,$(C_SRC))
CPP_OBJ  := $(patsubst src/cpp/%.cpp,$(BUILD)/cpp/%.o,$(CPP_SRC))

ifeq ($(SDL2),1)
CFLAGS   += $(shell pkg-config --cflags sdl2)
LDFLAGS  += $(shell pkg-config --libs sdl2)
CPPFLAGS += -DCB_USE_SDL2
endif

CPPFLAGS += -Ithird_party/lua-5.4 -Isrc/c

all: $(BIN)

$(BIN): $(LUA_OBJ) $(C_OBJ) $(CPP_OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

$(BUILD)/lua/%.o: third_party/lua-5.4/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/c/%.o: src/c/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD)/cpp/%.o: src/cpp/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN) --frames 90 --scene 1 --out out/frames --verbose

test: $(BIN)
	./tests/run_tests.sh

-include $(LUA_OBJ:.o=.d) $(C_OBJ:.o=.d) $(CPP_OBJ:.o=.d)

clean:
	rm -rf $(BUILD)

.PHONY: all run test clean
