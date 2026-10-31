# Salam Programming Language (2024-2026) - std/opencv native shim build
#
# Builds libsalam_opencv_shim (the library std/opencv/opencv.salam's
# `link dynamic "salam_opencv_shim"` expects to find). See BUILD.md for the
# full walkthrough, including how to make the result discoverable by
# `salam build`/`salam run`.
#
#   cmake -S native -B native/build -DCMAKE_BUILD_TYPE=Release
#   cmake --build native/build --config Release

cmake_minimum_required(VERSION 3.16)
project(salam_opencv_shim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV 4 REQUIRED COMPONENTS core imgproc imgcodecs features2d objdetect videoio highgui dnn)

add_library(salam_opencv_shim SHARED opencv_shim.cpp)
target_link_libraries(salam_opencv_shim PRIVATE ${OpenCV_LIBS})
target_include_directories(salam_opencv_shim PRIVATE ${OpenCV_INCLUDE_DIRS})

# A static variant, for embedding into a single binary instead of shipping
# a shared library alongside it (matches the `link static "..."` path in
# opencv.salam if you switch to it - see BUILD.md's "static instead of
# dynamic" note).
add_library(salam_opencv_shim_static STATIC opencv_shim.cpp)
target_link_libraries(salam_opencv_shim_static PRIVATE ${OpenCV_LIBS})
target_include_directories(salam_opencv_shim_static PRIVATE ${OpenCV_INCLUDE_DIRS})
set_target_properties(salam_opencv_shim_static PROPERTIES OUTPUT_NAME salam_opencv_shim)

install(TARGETS salam_opencv_shim salam_opencv_shim_static
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
