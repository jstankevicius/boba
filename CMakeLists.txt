
cmake_minimum_required (VERSION 3.14)

set (PROY stutter)
project (${PROY} LANGUAGES CXX)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -std=c++17 -Wall -Werror")

include_directories(${PROJECT_SOURCE_DIR}/src)

set(STUTTER_SRC
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

# Catch2
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
)

list(APPEND CMAKE_MODULE_PATH build/_deps/catch2-src/extras/)

FetchContent_MakeAvailable(Catch2)
enable_testing()

# Create and link executables
add_executable(stutter ${STUTTER_SRC})

add_executable(tests tests/test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

# Register tests
include(CTest)
include(Catch)
catch_discover_tests(tests)