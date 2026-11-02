cmake_minimum_required(VERSION 3.18)
project(salam_engine C)

if(NOT SALAM_ENGINE_GEN)
  if(ANDROID AND SALAM_ENGINE_GEN_ROOT)
    set(SALAM_ENGINE_GEN "${SALAM_ENGINE_GEN_ROOT}/${ANDROID_ABI}/gen")
  else()
    set(SALAM_ENGINE_GEN "${CMAKE_CURRENT_SOURCE_DIR}/build/gen")
  endif()
endif()

file(GLOB SALAM_GEN_SOURCES CONFIGURE_DEPENDS "${SALAM_ENGINE_GEN}/*.c")
if(NOT SALAM_GEN_SOURCES)
  message(FATAL_ERROR "no engine C in ${SALAM_ENGINE_GEN}; run tools/bash/build-mobile-engine.sh first")
endif()

add_library(salam_gen OBJECT ${SALAM_GEN_SOURCES})
target_include_directories(salam_gen PRIVATE "${SALAM_ENGINE_GEN}")
target_compile_definitions(salam_gen PRIVATE main=salam_engine_main)
set_target_properties(salam_gen PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(salam_gen PRIVATE -w -fno-strict-aliasing)
  if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    target_compile_options(salam_gen PRIVATE -fno-finite-loops)
  endif()
endif()

set(SALAM_ENGINE_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/salam_engine.c"
  $<TARGET_OBJECTS:salam_gen>)

if(ANDROID)
  add_library(salam SHARED ${SALAM_ENGINE_SOURCES}
    "${CMAKE_CURRENT_SOURCE_DIR}/salam_jni.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/android_compat.c")
  target_link_libraries(salam PRIVATE m dl log)
  target_link_options(salam PRIVATE -Wl,--gc-sections -Wl,-z,max-page-size=16384)
  target_compile_options(salam_gen PRIVATE -O2 -ffunction-sections -fdata-sections)
else()
  add_library(salam STATIC ${SALAM_ENGINE_SOURCES})
  find_package(Threads REQUIRED)
  target_link_libraries(salam PUBLIC m ${CMAKE_DL_LIBS} Threads::Threads)
  add_executable(salam_engine_test "${CMAKE_CURRENT_SOURCE_DIR}/test/engine_test.c")
  target_link_libraries(salam_engine_test PRIVATE salam)
endif()
target_include_directories(salam PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
