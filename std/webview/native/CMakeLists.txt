# Salam Programming Language (2024-2026)
#
# Builds the CEF backend of std/webview:
#
#   libsalam_webview_cef   - the shared library cef.salam links against
#   salam_cef_helper       - CEF's subprocess executable
#
# Both need the CEF binary distribution. See BUILD.md.
#
# Usage:
#   cmake -B build -DCEF_ROOT=/path/to/cef_binary_XXX
#   cmake --build build --config Release

cmake_minimum_required(VERSION 3.19)
project(salam_webview_cef CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT DEFINED CEF_ROOT)
    message(FATAL_ERROR
        "CEF_ROOT is not set. Point it at an unpacked CEF binary "
        "distribution: cmake -B build -DCEF_ROOT=/path/to/cef_binary_XXX")
endif()

# CEF ships its own CMake package: cmake/FindCEF.cmake plus the
# libcef_dll_wrapper target under libcef_dll/.
list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)
add_subdirectory("${CEF_ROOT}/libcef_dll" libcef_dll_wrapper)

# ------------------------------------------------------------------ shim --
add_library(salam_webview_cef SHARED cef_shim.cc)
target_include_directories(salam_webview_cef PRIVATE
    "${CEF_ROOT}"
    "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(salam_webview_cef PRIVATE
    libcef_lib
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS})
SET_LIBRARY_TARGET_PROPERTIES(salam_webview_cef)

# ---------------------------------------------------------------- helper --
# CEF re-executes this instead of the host program for its renderer, GPU and
# utility processes - which is what keeps CefExecuteProcess out of every Salam
# main(). cef_shim.cc finds it through $SALAM_CEF_HELPER.
add_executable(salam_cef_helper cef_helper.cc)
target_include_directories(salam_cef_helper PRIVATE "${CEF_ROOT}")
target_link_libraries(salam_cef_helper PRIVATE
    libcef_lib
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS})
SET_EXECUTABLE_TARGET_PROPERTIES(salam_cef_helper)

# ------------------------------------------------------------------ mock --
# Dependency-free stand-in used by tests/en/webview_cef; builds without CEF.
option(SALAM_WEBVIEW_CEF_MOCK "Also build the CEF-free mock" OFF)
if(SALAM_WEBVIEW_CEF_MOCK)
    add_library(salam_webview_cef_mock STATIC mock/cef_mock.c)
    target_include_directories(salam_webview_cef_mock PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
