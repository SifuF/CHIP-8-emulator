cmake_minimum_required(VERSION 3.22.1)

project(chip8 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(SOURCES
        src/main.cpp
        src/CHIP8.cpp
)

set(HEADERS
    src/CHIP8.hpp
    src/defs.hpp
    src/font.hpp
)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCES} ${HEADERS})

