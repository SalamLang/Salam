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

#ifndef _FILE_H_
#define _FILE_H_

#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <wchar.h>

#include "base.h"
#include "log.h"
#include "memory.h"
#include "string_buffer.h"

/**
 *
 * @function file_reads
 * @berif Reading entire of a file
 * @params {char*} path - Path of file
 * @params {size_t*} size - Size of file
 * @returns {char*} - Content of file
 *
 */
char *file_reads(const char *path, size_t *size);

/**
 *
 * @function file_writes
 * @berif Writing content to a file
 * @params {char*} path - Path of file
 * @params {char*} content - Content of file
 * @returns {bool}
 *
 */
bool file_writes(const char *path, const char *content);

/**
 *
 * @function file_appends
 * @brief Appending content to a file
 * @params {char*} path - Path of file
 * @params {char*} content - Content of file
 * @returns {bool}
 *
 */
bool file_appends(const char *path, const char *content);

/**
 *
 * @function file_exists
 * @berif Check if a file exists
 * @params {char*} path - Path of file
 * @returns {bool}
 *
 */
bool file_exists(const char *path);

/**
 *
 * @function file_remove
 * @berif Remove a file
 * @params {char*} path - Path of file
 * @returns {bool}
 *
 */
bool file_remove(const char *path);

/**
 *
 * @function file_copy
 * @berif Copy a file
 * @params {char*} source - Source file
 * @params {char*} destination - Destination file
 * @returns {bool}
 *
 */
bool file_copy(const char *source, const char *destination);

/**
 *
 * @function file_move
 * @berif Move a file
 * @params {char*} source - Source file
 * @params {char*} destination - Destination file
 * @returns {bool}
 *
 */
bool file_move(const char *source, const char *destination);

/**
 *
 * @function file_get_name
 * @berif Get the name of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Name of file
 *
 */
char *file_get_name(const char *path);

/**
 *
 * @function file_get_extension
 * @berif Get the extension of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Extension of file
 *
 */
char *file_get_extension(const char *path);

/**
 *
 * @function file_get_directory
 * @berif Get the directory of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Directory of file
 *
 */
char *file_get_directory(const char *path);

/**
 *
 * @function file_get_absolute
 * @brief Resolves the absolute path of a given relative path.
 * @params {const char*} path - The relative path.
 * @returns {char*} - The resolved absolute path, or NULL if an error occurred.
 *
 */
char *file_get_absolute(const char *path);

/**
 *
 * @function file_get_capacity
 * @berif Get the size of a file
 * @params {char*} path - Path of file
 * @returns {long} - Size of file
 *
 */
long file_get_capacity(const char *path);

/**
 *
 * @function file_get_modified
 * @berif Get the last modified time of a file
 * @params {char*} path - Path of file
 * @returns {time_t} - Last modified time of file
 *
 */
time_t file_get_modified(const char *path);

/**
 *
 * @function directory_create
 * @berif Create a directory
 * @params {char*} path - Path of directory
 * @returns {bool}
 *
 */
bool directory_exists(const char *path);

/**
 *
 * @function file_appends_wchar
 * @brief Appending wide character to a file
 * @params {char*} path - Path of file
 * @params {wchar_t} wc - Wide character
 * @returns {bool}
 *
 */
bool file_appends_wchar(const char *path, const wchar_t wc);

/**
 *
 * @function file_reads_binary
 * @brief Reading entire of a binary file
 * @params {char*} path - Path of file
 * @params {size_t*} size - Size of file
 * @returns {char*} - Content of file
 *
 */
char *file_reads_binary(const char *path, size_t *size);

#endif
