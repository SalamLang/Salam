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

#include "digit.h"

/**
 *
 * @function is_english_digit
 * @brief Check if the character is an English digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an English digit, false otherwise
 *
 */
bool is_english_digit(wchar_t ch) {
    DEBUG_ME;
    // 0123456789
    return ch >= L'0' && ch <= L'9';
}

/**
 *
 * @function is_persian_digit
 * @brief Check if the character is a Persian digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is a Persian digit, false otherwise
 *
 */
bool is_persian_digit(wchar_t ch) {
    DEBUG_ME;
    // ۰۱۲۳۴۵۶۷۸۹
    return ch >= 0x06F0 && ch <= 0x06F9;
    // return ch >= '۰' && ch <= '۹';
}

/**
 *
 * @function is_arabic_digit
 * @brief Check if the character is an Arabic digit
 * @params {wchar_t} ch - Character
 * @returns {bool} - True if the character is an Arabic digit, false otherwise
 *
 */
bool is_arabic_digit(wchar_t ch) {
    DEBUG_ME;
    // ٠١٢٣٤٥٦٧٨٩
    return ch >= 0x0660 && ch <= 0x0669;
}
