cmake_minimum_required(VERSION 3.10...3.20)

project(SableUI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenGL
find_package(OpenGL REQUIRED)

# glfw
add_subdirectory("vendor/glfw")

# Specify the path to GLEW
set(GLEW_DIR "${CMAKE_SOURCE_DIR}/vendor/glew")  # Path to GLEW
include_directories("${GLEW_DIR}/include")

add_definitions(-DGLEW_STATIC)

file(GLOB_RECURSE GLEW_SOURCE "${GLEW_DIR}/src/*.c" "${GLEW_DIR}/include/*.h")
add_library(glew ${GLEW_SOURCE})

# FreeType
set(FT_DISABLE_ZLIB     TRUE CACHE BOOL "Disable Zlib support in FreeType")
set(FT_DISABLE_BZIP2    TRUE CACHE BOOL "Disable BZip2 support in FreeType")
set(FT_DISABLE_PNG      TRUE CACHE BOOL "Disable PNG support in FreeType")
set(FT_DISABLE_HARFBUZZ TRUE CACHE BOOL "Disable HarfBuzz support in FreeType")
set(FT_DISABLE_BROTLI   TRUE CACHE BOOL "Disable Brotli support in FreeType")

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
    ${OPENGL_LIBRARIES}
    glew
    glfw
    TinyXML2
    freetype
)

target_include_directories(SableUI PRIVATE ${FREETYPE_INCLUDE_DIRS} "vendor/SDL/include")