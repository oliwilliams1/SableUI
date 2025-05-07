cmake_minimum_required(VERSION 3.5)

project(SableUI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SDL2
add_subdirectory("vendor/SDL" EXCLUDE_FROM_ALL)
include_directories("vendor/SDL/include")

# TinyXML2
file(GLOB_RECURSE TINYXML2_SOURCES "vendor/TinyXML2/*.cpp" "vendor/TinyXML2/*.h")

add_library(TinyXML2 STATIC ${TINYXML2_SOURCES})
target_include_directories(TinyXML2 PUBLIC "vendor/TinyXML2")

# SableUI
include_directories("include")
include_directories("SableUI")
file(GLOB_RECURSE SABLE_UI_SOURCES "SableUI/*.cpp" "SableUI/*.h" "include/*.h")

add_executable(SableUI ${SABLE_UI_SOURCES})

# Link libraries
target_link_libraries(SableUI PRIVATE "SDL2::SDL2-static" TinyXML2)