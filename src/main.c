#include <stdio.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <stdlib.h>

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

	lexer_t* lexer = lexer_create(path, content);
	lexer_lex(lexer);

	lexer_debug(lexer);

	lexer_save(lexer, "tokens.txt");

	// ast_t* ast = parser_parse(lexer);
	
	// ast_debug(ast);

	// printf("end ast debug\n");

	// generator_t* generator = generator_create(ast);

	// printf("generate code\n");

	// generator_code(generator);

	// printf("generate debug\n");

	// generator_debug(generator);

	// printf("generate save\n");

	// generator_save(generator, "index.html", "style.css", "script.js");

	// printf("generate destroy\n");
	// generator_destroy(generator);

	// printf("ast destroy\n");

	// ast_destroy(ast);

	// printf("end ast destroy\n");

	lexer_destroy(lexer);

	printf("end lexer destroy\n");

	memory_destroy(content);

	printf("end content destroy\n");

	printf("END SUCCESS\n");
}

int main(int argc, char** argv)
{
	DEBUG_ME;
    setlocale(LC_ALL, "");

	// array_value_t* values = array_value_create(1);
	// array_push(values, "12px");
	// ast_layout_attribute_t* attribute = ast_layout_attribute_create(AST_LAYOUT_ATTRIBUTE_TYPE_STYLE_FONT_SIZE, "font-size", values, AST_LAYOUT_NODE_TYPE_DIV, (location_t){0, 0, 0, 0, 0, 0}, (location_t){0, 0, 0, 0, 0, 0});
	// bool res = validate_style_value_size(attribute);
	// printf("validate style value size: %d\n", res);
	// attribute->print(attribute);

	doargs(argc, argv);

	return 0;
}
