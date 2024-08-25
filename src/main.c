#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdlib.h>
#include <iconv.h>

#include "base.h"
#include "file.h"
#include "memory.h"
#include "lexer.h"
#include "log.h"
#include "array.h"
#include "ast.h"
#include "parser.h"
#include "generator.h"
#include "validator.h"


void handle_character(unsigned char c)
{
    if (c == 0xB5) {
        printf("Encountered the micro sign (µ) at 0xB5\n");
    } else {
        printf("Valid character: %c\n", c);
    }
}

/**
 * 
 * @function handle_invalid_char
 * @brief Handle invalid characters in a string
 * @params {const unsigned char*} str - The string to check
 * @params {size_t} length - The length of the string
 * @returns {void}
 * 
 */
void handle_invalid_char(const unsigned char* str, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        if (str[i] == 0xEF && str[i + 1] == 0xBF && str[i + 2] == 0xBD) {
            printf("Replacement character detected at byte %zu\n", i);
            i += 2; // Skip over the replacement character sequence
        } else {
            printf("Valid byte: %02X\n", str[i]);
        }
    }
}

/**
 * 
 * @function doargs
 * @brief Handle command line arguments
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {void}
 * 
 */
void doargs(int argc, char** argv)
{
	DEBUG_ME;
	if (argc < 2) {
		error(1, "Usage: %s <file>\n", argv[0]);
	}

	const char* path = argv[1];
	if (!file_exists(path)) {
		error(1, "File does not exist: %s\n", path);
	}

	size_t size = 0;
	char* content = file_reads(path, &size);

	printf("LEXER SIZE: %zu\n", size);

	handle_invalid_char((const unsigned char*)content, size);

    size_t length = sizeof(content) / sizeof(content[0]);
    for (size_t i = 0; i < length; i++) {
        handle_character(content[i]);
    }

	lexer_t* lexer = lexer_create(path, content);
	lexer->source_size = size;
	lexer_lex(lexer);

	lexer_debug(lexer);

	lexer_save(lexer, "tokens.txt");

	ast_t* ast = parser_parse(lexer);

	ast_debug(ast);

	printf("end ast debug\n");

	generator_t* generator = generator_create(ast);

	printf("generate code\n");

	generator_code(generator);

	printf("generate debug\n");

	generator_debug(generator);

	printf("generate save\n");

	generator_save(generator, "index.html", "style.css", "script.js");
	// generator_save(generator, "../out/index.html", "../out/style.css", "../out/script.js");

	printf("generate destroy\n");
	generator_destroy(generator);

	printf("ast destroy\n");

	ast_destroy(ast);

	printf("end ast destroy\n");

	lexer_destroy(lexer);

	printf("end lexer destroy\n");

	memory_destroy(content);

	printf("end content destroy\n");

	printf("END SUCCESS\n");
}

/**
 * 
 * @function main
 * @brief Main entry point
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {int}
 * 
 */
int main(int argc, char** argv)
{
	DEBUG_ME;
    setlocale(LC_ALL, "");
    // setlocale(LC_ALL, "fa_IR.UTF-8");
    // setlocale(LC_ALL, "en_US.UTF-8");

	doargs(argc, argv);

	return 0;
}
