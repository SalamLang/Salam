cmake_minimum_required(VERSION 3.10)
project(Salam C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -g")

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.c)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(salam ${SOURCE_FILES})
