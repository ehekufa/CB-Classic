cmake_minimum_required(VERSION 3.16)
project(cb_classic C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(CB_USE_SDL2 "Собрать с оконным бэкендом SDL2" OFF)

file(GLOB LUA_SOURCES third_party/lua-5.4/*.c)
file(GLOB C_SOURCES src/c/*.c)
file(GLOB CPP_SOURCES src/cpp/*.cpp)

add_library(lua54 STATIC ${LUA_SOURCES})
target_include_directories(lua54 PUBLIC third_party/lua-5.4)
target_compile_options(lua54 PRIVATE -w)

add_library(cb_core STATIC ${C_SOURCES})
target_include_directories(cb_core PUBLIC src/c)

add_executable(cb-classic ${CPP_SOURCES})
target_include_directories(cb-classic PRIVATE src/c src/cpp third_party/stb third_party/minimp3)
target_link_libraries(cb-classic PRIVATE cb_core lua54 m)

if(CB_USE_SDL2)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SDL2 REQUIRED sdl2)
  target_compile_definitions(cb-classic PRIVATE CB_USE_SDL2)
  target_include_directories(cb-classic PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(cb-classic PRIVATE ${SDL2_LIBRARIES})
endif()

install(TARGETS cb-classic RUNTIME DESTINATION bin)
