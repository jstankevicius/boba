include(FetchContent)

cmake_minimum_required (VERSION 3.14)

set (PROY stutter)
project (${PROY} C CXX)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++17 -Wall -Werror")

include_directories (${PROJECT_SOURCE_DIR}/src)


set (STUTTER_SRC
    src/stutter.cpp
    src/lexer.cpp
    src/parser.cpp
    src/error.cpp
    src/analyzer.cpp
    src/bytecode.cpp
    src/runtime.cpp
)

# Output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)


add_executable (stutter ${STUTTER_SRC})