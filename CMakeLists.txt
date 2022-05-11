cmake_minimum_required (VERSION 3.14)

set (PROY stutter)
project (${PROY} C CXX)

# Source files folder
set (SRC_DIR src)

# Header files folder
set (INCL_DIR include)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

include_directories (${PROJECT_SOURCE_DIR}/${INCL_DIR})


set (STUTTER_SRC
    ${SRC_DIR}/stutter.cpp
    ${SRC_DIR}/lexer.cpp
)

# Output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

add_executable (stutter ${STUTTER_SRC})