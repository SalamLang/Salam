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

#include "main.h"

/**
 *
 * @function lint_do
 * @brief Linting the given content and parameters
 * @params {bool} isCode - Whether the content is code or file
 * @params {const char*} path - Path of the file
 * @params {char*} content - Content of the file
 * @params {char*} build_file - Build file
 * @returns {void}
 *
 */
void lint_do(bool isCode, const char *path, char *content, char *build_file) {
    DEBUG_ME;
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
    DEBUG_ME;
    if (content == NULL) {
        printf("Error: Content is NULL. Please provide valid content.\n");
        return;
    }

    lexer_t *lexer = lexer_create(path, content);

    lexer_lex(lexer);

    ast_t *ast = parser_parse(lexer);

    generator_t *generator = generator_create(ast);

    if (isCode == false && build_dir != NULL) {
        string_set_str(generator->output_dir, build_dir);
    }

    if (isCode == true) {
        generator->inlineCSS = true;
        generator->inlineJS = true;
    }

    generator_code(generator);

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
    DEBUG_ME;
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
 * @function update
 * @brief Update and download new version
 * @params {void}
 * @returns {void}
 *
 */
void update(int argc, char **argv)
{
    DEBUG_ME;
    if (argc) {}
    if (argv) {}

    printf("Check latest version...\n");

    const char *output_file = "update.tmp";
    const char *port = "80";
    const char *hostname = "versions.salamlang.ir";

    const char *path =
#ifdef _WIN32
"/latest/windows";
#elif __APPLE__
"/latest/macos";
#elif __linux__
"/latest/linux";
#elif __EMSCRIPTEN__
"/latest/webassembly";
#else
"Unsupported OS";
    printf("%s\n", path);
    exit(1);
#endif

    FILE *fp = fopen(output_file, "wb");

    printf("Connecting to the server...\n");
    bool res = download(fp, port, hostname, path);

    if (res == true) {
        printf("Download successful.\n");
    } else {
        printf("Download failed, something went wrong.\n");
    }
}

/**
 *
 * @function lint
 * @brief Lint a file or code based on arguments
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {void}
 *
 */
void lint(int argc, char **argv) {
    const char *path = argv[1];
    if (argc <= 2) {
        error(1, "Usage: %s lint <file>\n", argv[0]);
    }

    if (strcmp(argv[2], "code") == 0) {
        if (argc <= 3) {
            error(1, "Usage: %s lint code <content>\n", argv[0]);
        }

        char *content = argv[3];

        lint_do(true, "stdin", content, NULL);
    } else {
        if (!file_exists(argv[2])) {
            error(1, "Lint - File does not exist: %s\n", argv[2]);
        }

        if (argc <= 3) {
            error(1, "Usage: %s lint <file> <output>\n", argv[0]);
        }

        char *content = file_reads_binary(path, NULL);

        char *output_file = argv[3];

        lint_do(false, path, content, output_file);

        memory_destroy(content);
    }
}

/**
 *
 * @function code
 * @brief Execute code content
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {void}
 *
 */
void code(int argc, char **argv)
{
    if (argc <= 2) {
        error(1, "Usage: %s code <content>\n", argv[0]);
    }

    char *content = argv[2];

    char *output_dir = NULL;
    if (argc > 3) {
        output_dir = argv[3];
    }

    run(true, "stdin", content, output_dir);
}

/**
 *
 * @function execute
 * @brief Execute a file
 * @params {int} argc - Number of arguments
 * @params {char**} argv - Array of arguments
 * @returns {void}
 *
 */
void execute(int argc, char **argv)
{
    const char *path = argv[1];
    if (!file_exists(path)) {
        error(1, "Execute - File does not exist: %s\n", path);
    }

    char *content = file_reads_binary(path, NULL);

    char *output_dir = NULL;

    if (argc >= 3) {
        output_dir = argv[2];
    }

    run(false, path, content, output_dir);

    memory_destroy(content);
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
    // argc: ---3
    // ---./this.program
    // ---code
    // printf("argc: ---%d\n", argc);
    // printf("---%s\n", argv[0]);
    // printf("---%s\n", argv[1]);

    if (argc < 2) {
        help(argv[0]);
    }

    const char *path = argv[1];

    if (strcmp(path, "version") == 0) {
        printf("Salam %s\n", SALAM_VERSION);
        exit(1);
    } else if (strcmp(path, "help") == 0) {
        help(argv[0]);
    } else if (strcmp(path, "update") == 0) {
        update(argc, argv);
    } else if (strcmp(path, "lint") == 0) {
        lint(argc, argv);
    } else if (strcmp(path, "code") == 0) {
        code(argc, argv);
    } else {
        execute(argc, argv);
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
