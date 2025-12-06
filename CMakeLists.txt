cmake_minimum_required(VERSION 3.15)

project(SableUI VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if (MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# Find OpenGL
find_package(OpenGL REQUIRED)

# glfw
add_subdirectory("vendor/glfw")

# Specify the path to GLEW
set(GLEW_DIR "${CMAKE_SOURCE_DIR}/vendor/glew") # Path to GLEW
include_directories("${GLEW_DIR}/include")

add_definitions(-DGLEW_STATIC)

file(GLOB_RECURSE GLEW_SOURCE "${GLEW_DIR}/src/*.c" "${GLEW_DIR}/include/*.h")
add_library(glew STATIC ${GLEW_SOURCE})

# FreeType
set(FT_DISABLE_ZLIB			TRUE CACHE BOOL "Disable Zlib support in FreeType")
set(FT_DISABLE_BZIP2		TRUE CACHE BOOL "Disable BZip2 support in FreeType")
set(FT_DISABLE_PNG			TRUE CACHE BOOL "Disable PNG support in FreeType")
set(FT_DISABLE_HARFBUZZ		TRUE CACHE BOOL "Disable HarfBuzz support in FreeType")
set(FT_DISABLE_BROTLI		TRUE CACHE BOOL "Disable Brotli support in FreeType")

add_subdirectory("vendor/freetype" EXCLUDE_FROM_ALL)

# Stb
include_directories("vendor/stb")

# libwebp
set(WEBP_BUILD_ANIM_UTILS   OFF)
set(WEBP_BUILD_CWEBP        OFF)
set(WEBP_BUILD_DWEBP        OFF)
set(WEBP_BUILD_GIF2WEBP     OFF)
set(WEBP_BUILD_IMG2WEBP     OFF)
set(WEBP_BUILD_VWEBP        OFF)
set(WEBP_BUILD_WEBPINFO     OFF)
set(WEBP_BUILD_LIBWEBPMUX   OFF)
set(WEBP_BUILD_WEBPMUX      OFF)
set(WEBP_BUILD_EXTRAS       OFF)
set(WEBP_USE_THREAD         OFF)
set(WEBP_NEAR_LOSSLESS      OFF)
set(WEBP_UNICODE            OFF)
add_subdirectory("vendor/libwebp")
include_directories("vendor/libwebp/src")

# SableUI
include_directories("include")
include_directories("SableUI")
file(GLOB_RECURSE SABLE_UI_SOURCES "SableUI/*.cpp" "SableUI/*.h" "include/*.h")

# Embed shaders
set(SHADER_DIR ${CMAKE_SOURCE_DIR}/shaders)
set(GENERATED_SHADER_HEADER "${CMAKE_SOURCE_DIR}/include/SableUI/generated/shaders.h")

file(GLOB SHADER_FILES "${SHADER_DIR}/*")
file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/include/SableUI/generated")
file(WRITE ${GENERATED_SHADER_HEADER} "// Auto-generated shader header\n#pragma once\n\n")

foreach(SHADER_FILE ${SHADER_FILES})
	get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
	string(REPLACE "." "_" VAR_NAME ${SHADER_NAME}) # shader.frag -> shader_frag

	file(READ ${SHADER_FILE} SHADER_CONTENTS)

	file(APPEND ${GENERATED_SHADER_HEADER}
		"constexpr const char ${VAR_NAME}[] = R\"(${SHADER_CONTENTS})\";\n\n"
	)
endforeach()

add_custom_target(EmbedShaders
	COMMENT "Embedding shaders into ${GENERATED_SHADER_HEADER}"
)

add_executable(SableUI ${SABLE_UI_SOURCES})
add_dependencies(SableUI EmbedShaders)

if (MSVC)
	set_property(TARGET SableUI PROPERTY WIN32_EXECUTABLE "$<$<CONFIG:Release>:TRUE>$<$<NOT:$<CONFIG:Release>>:FALSE>")
	target_link_options(SableUI PRIVATE $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
endif()

# Link libraries
target_link_libraries(SableUI PRIVATE
	${OPENGL_LIBRARIES}
	glew
	glfw
	freetype
	webp
)

if(UNIX)
	target_link_libraries(SableUI PRIVATE GL)
endif()

target_include_directories(SableUI PRIVATE ${FREETYPE_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include/SableUI/generated")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/fonts/" 
     DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/fonts/")
