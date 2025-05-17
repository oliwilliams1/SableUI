cmake_minimum_required(VERSION 3.10)

project(SableUI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# SDL2
add_subdirectory("vendor/SDL" EXCLUDE_FROM_ALL)

# FreeType
set(FT_DISABLE_ZLIB TRUE CACHE BOOL "Disable Zlib support in FreeType")
set(FT_DISABLE_BZIP2 TRUE CACHE BOOL "Disable BZip2 support in FreeType")
set(FT_DISABLE_PNG TRUE CACHE BOOL "Disable PNG support in FreeType")
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "Disable HarfBuzz support in FreeType")
set(FT_DISABLE_BROTLI TRUE CACHE BOOL "Disable Brotli support in FreeType")

add_subdirectory("vendor/freetype" EXCLUDE_FROM_ALL)

# Stb
include_directories("vendor/stb")

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
target_link_libraries(SableUI PRIVATE 
    "SDL2::SDL2-static" 
    TinyXML2 
    freetype
)

target_include_directories(SableUI PRIVATE ${FREETYPE_INCLUDE_DIRS} "vendor/SDL/include")