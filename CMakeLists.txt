cmake_minimum_required(VERSION 3.5)

project(SableUI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory("vendor/SDL2" EXCLUDE_FROM_ALL)

include_directories(${SDL2_INCLUDE_DIRS})

file(GLOB_RECURSE SABLE_UI_SOURCES "src/*.cpp" "src/*.h")

if(NOT SABLE_UI_SOURCES)
    message(FATAL_ERROR "No source files found in 'src' directory.")
endif()

add_executable(SableUI ${SABLE_UI_SOURCES})

if(TARGET SDL2::SDL2main)
    target_link_libraries(SableUI PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(SableUI PRIVATE "SDL2::SDL2-static")