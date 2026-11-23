# Сборка нативной части APK: движок (C/C++) + Lua + SDL2.
cmake_minimum_required(VERSION 3.18)
project(cbclassic C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(CB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)

if(NOT DEFINED SDL2_SOURCE_DIR)
    set(SDL2_SOURCE_DIR "${CB_ROOT}/third_party/SDL")
endif()

set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory("${SDL2_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/sdl2")

# Lua 5.4 (вендорен)
file(GLOB LUA_SOURCES "${CB_ROOT}/third_party/lua-5.4/*.c")
add_library(lua54 STATIC ${LUA_SOURCES})
target_include_directories(lua54 PUBLIC "${CB_ROOT}/third_party/lua-5.4")
target_compile_options(lua54 PRIVATE -w)

# C-часть движка: inflate, PNG, ввод-вывод
file(GLOB C_SOURCES "${CB_ROOT}/src/c/*.c")
add_library(cb_core STATIC ${C_SOURCES})
target_include_directories(cb_core PUBLIC "${CB_ROOT}/src/c")

# C++-часть + SDL2-бэкенд; имя библиотеки "main" требует SDLActivity
file(GLOB CPP_SOURCES "${CB_ROOT}/src/cpp/*.cpp")
add_library(main SHARED ${CPP_SOURCES})
target_include_directories(main PRIVATE "${CB_ROOT}/src/c" "${CB_ROOT}/src/cpp")
target_compile_definitions(main PRIVATE CB_USE_SDL2)
target_link_libraries(main cb_core lua54 SDL2main SDL2 log m)
