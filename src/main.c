#include "main.h"

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
void lint(bool isCode, const char *path, char *content, char *build_file) {
    lexer_t *lexer = lexer_create(path, content);

    lexer_lex(lexer);

    ast_t *ast = parser_parse(lexer);

    generator_t *generator = generator_create(ast);

    generator_code(generator);

    string_t *cleaned_code = generator_salam(ast);

    if (build_file != NULL) {
        file_writes(build_file, cleaned_code->data);
    } else {
        printf("%s\n", cleaned_code->data);
    }

    generator_destroy(generator);

    string_destroy(cleaned_code);

    if (!isCode) {
        printf("END SUCCESS\n");
    }

    ast_destroy(ast);

    lexer_destroy(lexer);
}

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
void run(bool isCode, const char *path, char *content, char *build_dir) {
    lexer_t *lexer = lexer_create(path, content);

    lexer_lex(lexer);

    // lexer_debug(lexer);

    // lexer_save(lexer, "tokens.txt");

    ast_t *ast = parser_parse(lexer);

    // ast_debug(ast);

    generator_t *generator = generator_create(ast);

    if (isCode == false && build_dir != NULL) {
        string_set_str(generator->output_dir, build_dir);
    }

    if (isCode == true) {
        generator->inlineCSS = true;
        generator->inlineJS = true;
    }

    generator_code(generator);

    // generator_debug(generator);

    if (isCode == true) {
        if (build_dir == NULL) {
            printf("%s\n", generator->html->data);
        } else {
            file_writes(build_dir, generator->html->data);
        }
    } else {
        generator_save(generator, "index.html", "style.css", "script.js");
    }

    generator_destroy(generator);

    ast_destroy(ast);

    lexer_destroy(lexer);

    if (!isCode) {
        printf("END SUCCESS\n");
    }
}

/**
 *
 * @function help
 * @brief Display help message
 * @params {char*} app - Application name
 * @returns {void}
 *
 */
void help(char *app) {
    printf("Welcome to Salam Programming Language!\n");
    printf(
        "Salam is the first Persian/Iranian computer scripting "
        "language.\n");
    printf("\n");
    printf("Usage:\n");
    printf("%s <filename>                      # Execute a Salam script\n",
           app);
    printf(
        "%s code <content> <output_dir>     # Compile and run Salam "
        "code\n",
        app);
    printf("\n");
    printf("%s lint <filename> <output_dir>    # Lint a Salam script\n", app);
    printf("%s lint code <content>             # Lint Salam code\n", app);
    printf("\n");
    printf(
        "%s version                         # Print the version of "
        "Salam\n",
        app);
    printf("\n");
    printf(
        "%s update                          # Update Salam to the "
        "latest version\n",
        app);

    printf("\n");
    printf("Feel free to explore and create using Salam!\n");
    printf("\n");
    printf("For more information, visit: https://salamlang.ir\n");

    exit(1);
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
void doargs(int argc, char **argv) {
    DEBUG_ME;
    if (argc < 2) {
        help(argv[0]);
    }

    const char *path = argv[1];
    // printf("path: %s\n", path);
    // printf("Argc: %d\n", argc);

    if (strcmp(path, "version") == 0) {
        printf("Salam %s\n", SALAM_VERSION);
        exit(1);
    } else if (strcmp(path, "help") == 0) {
        help(argv[0]);
    } else if (strcmp(path, "update") == 0) {
        printf("TODO: auto update feature...\n");
    } else if (strcmp(path, "lint") == 0) {
        if (argc <= 2) {
            error(1, "Usage: %s lint <file>\n", argv[0]);
        }

        if (strcmp(argv[2], "code") == 0) {
            if (argc <= 3) {
                error(1, "Usage: %s lint code <content>\n", argv[0]);
            }

            char *content = argv[3];

            lint(true, "stdin", content, NULL);
        } else {
            if (!file_exists(argv[2])) {
                error(1, "File does not exist: %s\n", argv[2]);
            }

            if (argc <= 3) {
                error(1, "Usage: %s lint <file> <output>\n", argv[0]);
            }

            char *content = file_reads_binary(path, NULL);

            char *output_file = argv[3];

            lint(false, path, content, output_file);

            memory_destroy(content);
        }
    } else if (strcmp(path, "code") == 0) {
        if (argc <= 2) {
            error(1, "Usage: %s code <content>\n", argv[0]);
        }

        char *content = argv[2];

        char *output_dir = argv[3];

        run(true, "stdin", content, output_dir);
    } else {
        if (!file_exists(path)) {
            error(1, "File does not exist: %s\n", path);
        }

        char *content = file_reads_binary(path, NULL);

        char *output_dir = NULL;

        if (argc >= 3) {
            output_dir = argv[2];
        }

        run(false, path, content, output_dir);

        memory_destroy(content);
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
int main(int argc, char **argv) {
    DEBUG_ME;
    setlocale(LC_ALL, "");
    // setlocale(LC_ALL, "fa_IR.UTF-8");
    // setlocale(LC_ALL, "en_US.UTF-8");

    doargs(argc, argv);

    // #ifdef __EMSCRIPTEN__
    //     emscripten_force_exit(0);
    // #endif

    return 0;
}
