cmake_minimum_required(VERSION 3.16)
project(chip8 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(SDL2 REQUIRED)

add_library(chip8core STATIC src/chip8.cpp)
target_include_directories(chip8core PUBLIC src)
if(MSVC)
  target_compile_options(chip8core PRIVATE /W4)
else()
  target_compile_options(chip8core PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(chip8 src/main.cpp)
target_link_libraries(chip8 PRIVATE chip8core SDL2::SDL2)

enable_testing()
add_executable(chip8_tests tests/tests.cpp)
target_link_libraries(chip8_tests PRIVATE chip8core)
add_test(NAME opcodes COMMAND chip8_tests)
