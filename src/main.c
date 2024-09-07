#include "main.h"

/**
 *
 * @function run
 * @brief
 */
void run(bool isCode, const char *path, char *content, char *build_dir)
{

	lexer_t *lexer = lexer_create(path, content);
	lexer_lex(lexer);

	// lexer_debug(lexer);

	// lexer_save(lexer, "tokens.txt");

	ast_t *ast = parser_parse(lexer);

	// ast_debug(ast);

	// printf("end ast debug\n");

	generator_t *generator = generator_create(ast);

	if (build_dir != NULL)
	{
		string_set_str(generator->output_dir, build_dir);
	}

	// printf("generate code\n");

	generator_code(generator);

	// printf("generate debug\n");

	// generator_debug(generator);

	// printf("generate save\n");

	// generator_save(generator, argc > 2 ? argv[2] : "index.html", argc > 3 ? argv[3] : "style.css", argc > 4 ? argv[4] : "script.js");
	generator_save(generator, "index.html", "style.css", "script.js");

	if (isCode)
	{
		if (generator->css->length > 0 || generator->media_css->length > 0)
		{
			printf("<style>\n");

			if (generator->css->length > 0)
			{
				printf("%s\n", generator->css->data);
			}

			if (generator->media_css->length > 0)
			{
				printf("%s\n", generator->media_css->data);
			}

			printf("</style>\n\n");
		}

		printf("%s\n", generator->html->data);

		if (generator->js->length > 0)
		{
			printf("%s\n", generator->js->data);
		}
	}

	// printf("generate destroy\n");
	generator_destroy(generator);

	// printf("ast destroy\n");

	ast_destroy(ast);

	// printf("end ast destroy\n");

	lexer_destroy(lexer);

	// printf("end lexer destroy\n");

	memory_destroy(content);

	// printf("end content destroy\n");

	if (!isCode)
	{
		printf("END SUCCESS\n");
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
void doargs(int argc, char **argv)
{
	DEBUG_ME;
	if (argc < 2)
	{
		error(1, "Usage: %s <file>\n", argv[0]);
	}

	const char *path = argv[1];

	if (strcmp(path, "version") == 0)
	{
		printf("Salam %s\n", SALAM_VERSION);
		exit(1);
	}
	else if (strcmp(path, "update") == 0)
	{
		printf("TODO: auto update feature...\n");
	}
	else if (strcmp(path, "code") == 0)
	{
		if (argc == 2)
		{
			error(1, "You need to pass filename as an extra input!\n");
		}

		char *content = argv[2];

		char *build_dir = argc > 3 ? argv[3] : NULL;

		run(true, "stdin", content, build_dir);
	}
	else
	{
		if (!file_exists(path))
		{
			error(1, "File does not exist: %s\n", path);
		}

		char *content = file_reads_binary(path, NULL);

		char *build_dir = argc > 2 ? argv[2] : NULL;

		run(false, path, content, build_dir);
	}
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
int main(int argc, char **argv)
{
	DEBUG_ME;
	setlocale(LC_ALL, "");
	// setlocale(LC_ALL, "fa_IR.UTF-8");
	// setlocale(LC_ALL, "en_US.UTF-8");

	doargs(argc, argv);

	return 0;
}
