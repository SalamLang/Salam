#ifndef _MAIN_H
#define _MAIN_H

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

#include "array.h"
#include "ast.h"
#include "base.h"
#include "downloader.h"
#include "file.h"
#include "generator.h"
#include "generator_salam.h"
#include "lexer.h"
#include "log.h"
#include "memory.h"
#include "parser.h"
#include "validator.h"
#include "validator_style.h"

/**
 *
 * @function lint
 * @brief Linting the given content and parameters
 * @params {bool} isCode - Whether the content is code or file
 * @params {const char*} path - Path of the file
 * @params {char*} content - Content of the file
 * @params {char*} build_file - Build file
 * @returns {void}
 *
 */
void lint(bool isCode, const char *path, char *content, char *build_file);

/**
 *
 * @function run
 * @brief Running the compiler with the given content and parameters
 * @params {bool} isCode - Whether the content is code or file
 * @params {const char*} path - Path of the file
 * @params {char*} content - Content of the file
 * @params {char*} build_dir - Build directory
 * @returns {void}
 *
 */
void run(bool isCode, const char *path, char *content, char *build_dir);

/**
 *
 * @function help
 * @brief Display help message
 * @params {char*} app - Application name
 * @returns {void}
 *
 */
void help(char *app);

/**
 * 
 * @function update
 * @brief Update and download new version
 * @params {void}
 * @returns {void}
 * 
 */
void update();

/**
 *
 * @function doargs
 * @brief Handle command line arguments
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {void}
 *
 */
void doargs(int argc, char **argv);

/**
 *
 * @function main
 * @brief Main entry point
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {int}
 *
 */
int main(int argc, char **argv);

#endif
