#
# Salam compiler - CMake build.
#
# Quick start (native):
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j
#   ./build/salam version
#
# WebAssembly playground bundle:
#   emcmake cmake -S . -B build-wasm
#   cmake --build build-wasm
#

cmake_minimum_required(VERSION 3.16)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/../VERSION" SALAM_VERSION_RAW LIMIT_COUNT 1)
string(STRIP "${SALAM_VERSION_RAW}" SALAM_VERSION_RAW)
if(NOT SALAM_VERSION_RAW)
    set(SALAM_VERSION_RAW "0.0.0")
endif()
string(REGEX MATCHALL "[0-9]+" SALAM_VERSION_PARTS "${SALAM_VERSION_RAW}")
list(LENGTH SALAM_VERSION_PARTS SALAM_VERSION_NPARTS)
while(SALAM_VERSION_NPARTS LESS 3)
    list(APPEND SALAM_VERSION_PARTS "0")
    list(LENGTH SALAM_VERSION_PARTS SALAM_VERSION_NPARTS)
endwhile()
list(GET SALAM_VERSION_PARTS 0 1 2 SALAM_VERSION_TRIMMED)
string(REPLACE ";" "." SALAM_VERSION "${SALAM_VERSION_TRIMMED}")

project(Salam
    VERSION ${SALAM_VERSION}
    DESCRIPTION "The Salam programming language compiler"
    LANGUAGES C)

get_property(_is_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _is_multi_config AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

option(SALAM_BUILD_TESTS "Enable the `test` target (runs tools/bash/run-tests.sh)" ON)

set(SALAM_SRC_DIRS
    core source condcomp logger xml diag i18n langpack
    token lexer ast parser semantic codegen jsgen llvm interp
    layout fmt cli driver)

set(SALAM_SOURCES "")
foreach(_dir IN LISTS SALAM_SRC_DIRS)
    file(GLOB _dir_srcs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/${_dir}/*.c")
    list(APPEND SALAM_SOURCES ${_dir_srcs})
endforeach()

if(NOT SALAM_SOURCES)
    message(FATAL_ERROR "No Salam sources found under src/ - is the source tree intact?")
endif()

add_executable(salam
    ${SALAM_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/src/salam.c")

target_include_directories(salam PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src")

set_target_properties(salam PROPERTIES
    C_STANDARD 90
    C_EXTENSIONS ON)

set(_is_tcc FALSE)
if(CMAKE_C_COMPILER MATCHES "tcc")
    set(_is_tcc TRUE)
endif()

find_package(Git QUIET)
set(SALAM_GIT_COMMIT "")
set(SALAM_GIT_DATE "")
set(SALAM_GIT_DIRTY "")
if(Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-parse --short HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE SALAM_GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" show -s --format=%cs HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE SALAM_GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" status --porcelain
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        OUTPUT_VARIABLE SALAM_GIT_STATUS
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
    if(NOT SALAM_GIT_STATUS STREQUAL "")
        set(SALAM_GIT_DIRTY "-dirty")
    endif()
endif()
if(SALAM_GIT_COMMIT)
    target_compile_definitions(salam PRIVATE
        SALAM_GIT_COMMIT="${SALAM_GIT_COMMIT}"
        SALAM_GIT_DATE="${SALAM_GIT_DATE}"
        SALAM_GIT_DIRTY="${SALAM_GIT_DIRTY}")
    message(STATUS "Salam: git ${SALAM_GIT_COMMIT}${SALAM_GIT_DIRTY} (${SALAM_GIT_DATE})")
endif()

if(MSVC)
    target_compile_options(salam PRIVATE /W3 /wd4244 /wd4267 /wd4101)
    target_compile_definitions(salam PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(salam PRIVATE
        -Wall -Wextra
        -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable)
endif()

if(NOT _is_tcc AND NOT EMSCRIPTEN AND NOT MSVC)
    target_link_libraries(salam PRIVATE m)
endif()

if(EMSCRIPTEN)
    file(GLOB _web_srcs CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/web/*.c")
    target_sources(salam PRIVATE ${_web_srcs})

    set_target_properties(salam PROPERTIES
        OUTPUT_NAME "salam-wa"
        SUFFIX ".js")

    target_compile_options(salam PRIVATE -O2)
    target_link_options(salam PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/std@/std"
        "-sMODULARIZE=0"
        "-sENVIRONMENT=web,worker,node"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sINITIAL_MEMORY=33554432"
        "-sSTACK_SIZE=4194304"
        "-sEXIT_RUNTIME=0"
        "-sIGNORE_MISSING_MAIN=1"
        "-sFILESYSTEM=1"
        "SHELL:-sEXPORTED_FUNCTIONS=['_salam_web_run_app','_salam_web_build_layout','_salam_web_emit','_salam_web_version','_malloc','_free']"
        "SHELL:-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8','lengthBytesUTF8','FS']")
endif()

include(GNUInstallDirs)
install(TARGETS salam RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/std")
    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/std"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/salam")
endif()

option(SALAM_BUNDLE_MINGW
    "Stage a mingw-w64 sysroot into share/salam/sysroots for Windows cross-compilation" OFF)
set(SALAM_MINGW_ARCH "x86_64" CACHE STRING
    "Architecture of the bundled mingw sysroot (x86_64|i686|aarch64|armv7)")
set(SALAM_MINGW_SYSROOT "" CACHE PATH
    "Path to an existing mingw-w64 sysroot to copy (skips downloading)")
set(SALAM_MINGW_URL "" CACHE STRING
    "URL of a mingw-w64 / llvm-mingw archive to download and extract")
set(SALAM_MINGW_SHA256 "" CACHE STRING
    "Expected SHA256 of SALAM_MINGW_URL (optional but recommended)")

if(SALAM_BUNDLE_MINGW)
    set(_mingw_sub "${SALAM_MINGW_ARCH}-w64-mingw32")
    set(_mingw_src "")

    if(SALAM_MINGW_SYSROOT)
        if(NOT EXISTS "${SALAM_MINGW_SYSROOT}/include/windows.h")
            message(FATAL_ERROR
                "SALAM_MINGW_SYSROOT='${SALAM_MINGW_SYSROOT}' is not a mingw sysroot "
                "(no include/windows.h)")
        endif()
        set(_mingw_src "${SALAM_MINGW_SYSROOT}")
    elseif(SALAM_MINGW_URL)
        set(_dl "${CMAKE_BINARY_DIR}/_mingw_dl.archive")
        if(NOT EXISTS "${_dl}")
            message(STATUS "Salam: downloading mingw sysroot from ${SALAM_MINGW_URL}")
            if(SALAM_MINGW_SHA256)
                file(DOWNLOAD "${SALAM_MINGW_URL}" "${_dl}" SHOW_PROGRESS
                    EXPECTED_HASH SHA256=${SALAM_MINGW_SHA256} STATUS _dlst)
            else()
                message(WARNING "Salam: SALAM_MINGW_SHA256 unset - download is unverified")
                file(DOWNLOAD "${SALAM_MINGW_URL}" "${_dl}" SHOW_PROGRESS STATUS _dlst)
            endif()
            list(GET _dlst 0 _dlcode)
            if(NOT _dlcode EQUAL 0)
                file(REMOVE "${_dl}")
                message(FATAL_ERROR "Salam: mingw sysroot download failed: ${_dlst}")
            endif()
        endif()
        set(_ex "${CMAKE_BINARY_DIR}/_mingw_extract")
        file(REMOVE_RECURSE "${_ex}")
        file(MAKE_DIRECTORY "${_ex}")
        execute_process(
            COMMAND "${CMAKE_COMMAND}" -E tar xf "${_dl}"
            WORKING_DIRECTORY "${_ex}" RESULT_VARIABLE _exok)
        if(NOT _exok EQUAL 0)
            message(FATAL_ERROR "Salam: failed to extract '${_dl}'")
        endif()
        file(GLOB_RECURSE _cand "${_ex}/*/${_mingw_sub}/include/windows.h")
        if(NOT _cand)
            file(GLOB_RECURSE _cand "${_ex}/${_mingw_sub}/include/windows.h")
        endif()
        if(NOT _cand)
            message(FATAL_ERROR
                "Salam: '${_mingw_sub}/include/windows.h' not found in the archive")
        endif()
        list(GET _cand 0 _hdr)
        get_filename_component(_incdir "${_hdr}" DIRECTORY)
        get_filename_component(_mingw_src "${_incdir}" DIRECTORY)
    else()
        message(FATAL_ERROR
            "SALAM_BUNDLE_MINGW=ON requires SALAM_MINGW_SYSROOT=<dir> or "
            "SALAM_MINGW_URL=<archive url>")
    endif()

    install(DIRECTORY "${_mingw_src}/"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/salam/sysroots/${_mingw_sub}"
        USE_SOURCE_PERMISSIONS)
    message(STATUS "Salam: bundling mingw sysroot '${_mingw_sub}' from ${_mingw_src}")
endif()

find_program(SALAM_SH NAMES sh bash)

if(SALAM_BUILD_TESTS AND SALAM_SH)
    add_custom_target(run-tests
        COMMAND "${SALAM_SH}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/bash/run-tests.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS salam
        USES_TERMINAL
        COMMENT "Running Salam end-to-end test suite")

    enable_testing()
    add_test(NAME salam-suite
        COMMAND "${SALAM_SH}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/bash/run-tests.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set_tests_properties(salam-suite PROPERTIES
        ENVIRONMENT "SALAM=$<TARGET_FILE:salam>")
endif()

if(SALAM_SH)
    add_custom_target(dist
        COMMAND "${SALAM_SH}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/bash/build-release.sh"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS salam
        USES_TERMINAL
        COMMENT "Staging a distributable release into dist/")
endif()

message(STATUS "Salam ${SALAM_VERSION} - C compiler: ${CMAKE_C_COMPILER_ID} (${CMAKE_C_COMPILER})")
list(LENGTH SALAM_SOURCES _n_srcs)
message(STATUS "Salam: ${_n_srcs} library sources + src/salam.c")
if(EMSCRIPTEN)
    message(STATUS "Salam: WebAssembly build enabled (output salam-wa.js)")
endif()
