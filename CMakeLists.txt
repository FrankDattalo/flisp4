cmake_minimum_required(VERSION 3.10.2)
project(flang)
include_directories(include)
set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_CPP_STANDARD 20)
set(CMAKE_CPP_FLAGS "-Wall -Wextra -Wpedantic -Werror -pipe -fconcepts")
set(SOURCES
  ${PROJECT_SOURCE_DIR}/src/objects/assert.cpp
  ${PROJECT_SOURCE_DIR}/src/objects/env.cpp
  ${PROJECT_SOURCE_DIR}/src/objects/map.cpp
  ${PROJECT_SOURCE_DIR}/src/objects/pair.cpp
  ${PROJECT_SOURCE_DIR}/src/objects/primitive.cpp
  ${PROJECT_SOURCE_DIR}/src/objects/slotiter.cpp
  ${PROJECT_SOURCE_DIR}/src/objects/stack.cpp
  ${PROJECT_SOURCE_DIR}/src/heap.cpp
)
add_executable(flang
  ${PROJECT_SOURCE_DIR}/src/main.cpp
  ${SOURCES})
target_compile_features(flang PRIVATE cxx_std_20)