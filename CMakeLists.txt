cmake_minimum_required(VERSION 3.10)
project(Salam C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -g)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(${SRC_DIR})

file(GLOB_RECURSE SRC_FILES ${SRC_DIR}/*.c)

set(BIN_DIR ${CMAKE_BINARY_DIR}/build)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR})

add_executable(salam ${SRC_FILES})

add_custom_command(
    OUTPUT ${BIN_DIR}/salam.bin
    COMMAND ${CMAKE_OBJCOPY} -O binary salam ${BIN_DIR}/salam.bin
    DEPENDS salam
    COMMENT "Generating salam.bin from executable"
)

add_custom_target(bin ALL DEPENDS ${BIN_DIR}/salam.bin)

add_custom_target(test
    COMMAND ${BIN_DIR}/salam ${SRC_DIR}/input.salam
    DEPENDS salam
    COMMENT "Running salam with input.salam"
)
