cmake_minimum_required(VERSION 3.5)

project(SableUI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SDL2
add_subdirectory("vendor/SDL" EXCLUDE_FROM_ALL)
include_directories("vendor/SDL/include")

# SableUI
include_directories("include")
include_directories("SableUI")
file(GLOB_RECURSE SABLE_UI_SOURCES "SableUI/*.cpp" "SableUI/*.h" "include/*.h")

add_executable(SableUI ${SABLE_UI_SOURCES})

# SDL2
target_link_libraries(SableUI PRIVATE "SDL2::SDL2-static")