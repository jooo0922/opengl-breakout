cmake_minimum_required(VERSION 3.18)

set(TARGET_NAME "breakout")

project(TARGET_NAME)
set(PROJECT_DESCRIPTION "A recreation of the classic Atari 2600 Breakout game using OpenGL")

# ----------------------------------------------------------------------------
# Configure build flags
# ----------------------------------------------------------------------------
set(LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------------------
# compile option
# ----------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # MSVC Compiler
  add_compile_options("/utf-8")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # GCC Compiler
  add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # Clang Compiler
  add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  # AppleClang Compiler (macOS)
  add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8")
endif()

# ----------------------------------------------------------------------------
# Directories
# ----------------------------------------------------------------------------
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/3rdparty")
set(CMAKE_DIR "${CMAKE_SOURCE_DIR}/_cmake")

# ----------------------------------------------------------------------------
# subs cmake (dependency library)
# ----------------------------------------------------------------------------
include(${CMAKE_DIR}/glfw.cmake)
include(${CMAKE_DIR}/glm.cmake)
include(${CMAKE_DIR}/stb.cmake)

# ----------------------------------------------------------------------------
# files
# ----------------------------------------------------------------------------
add_executable(${TARGET_NAME}

  # glad
  ${SRC_DIR}/glad.c

  # current src
  ${SRC_DIR}/game/game.cpp

  ${SRC_DIR}/manager/resource_manager.cpp

  ${SRC_DIR}/renderer/sprite_renderer.cpp

  ${SRC_DIR}/utils/shader.cpp
  ${SRC_DIR}/utils/texture.cpp

  # current main
  ${SRC_DIR}/main.cpp
)

target_include_directories(${TARGET_NAME}
  PRIVATE
  ${THIRDPARTY_DIR}
  ${glfw_INCLUDE}
  ${glm_INCLUDE}
  ${stb_INCLUDE}
)

target_link_libraries(${TARGET_NAME}
  PRIVATE
  glfw
)
