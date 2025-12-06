/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 *
 * Welcome to the Salam Programming Language! Salam is a modern, efficient,
 * and developer-friendly language created to inspire and empower programmers
 * of all ages. With its unique features and Persian roots, Salam aims to make
 * programming more accessible and enjoyable for everyone.
 *
 * This file is part of the core implementation of Salam, including its runtime
 * and compiler components. By contributing to or using this codebase, you are
 * part of a growing community dedicated to innovation and inclusivity in
 * programming.
 *
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 *
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

#ifndef _BASE_H_
#define _BASE_H_

// #ifdef __EMSCRIPTEN__
// #include <emscripten/emscripten.h>
// #endif

#define cast(TYPE, VALUE) ((TYPE)VALUE)

#define _CRT_NONSTDC_NO_DEPRECATE

#define GRAY "\033[90m"
#define BLACK "\033[30m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"
#define WHITE "\033[37m"
#define RESET "\033[0m"

#define BRIGHT_BLACK "\033[90m"
#define BRIGHT_RED "\033[91m"
#define BRIGHT_GREEN "\033[92m"
#define BRIGHT_YELLOW "\033[93m"
#define BRIGHT_BLUE "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN "\033[96m"
#define BRIGHT_WHITE "\033[97m"

#include <stdbool.h>  // for bool, true, false
#include <stddef.h>   // for size_t
#include <stdio.h>    // for printf

#define DEBUG_ME \
    printf(GRAY "DEBUG: %s:%d in %s\n" RESET, __FILE__, __LINE__, __func__)
// #define DEBUG_ME

#define SALAM_VERSION "0.1"

// #define _XOPEN_SOURCE 700

#include <base_all.h>

#endif
