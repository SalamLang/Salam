#include "main.h"

void print_help() {
    DEBUG_ME;
    printf("Usage:\n");
    printf("  ./s file <filename>      Run code from a file\n");
    printf("  ./s code <code inline>   Run inline code\n");
    printf("  ./s --lang <language>     Set the language (en, fa)\n");
    printf("  ./s --help               Show this help message\n");
    printf("  ./s --version            Show the program version\n");
}

void print_version() {
    DEBUG_ME;
    printf("Version: %s\n", SALAM_VERSION);
}

int run_code(const char *source, const char *filepath, language_map_t selected_language) {
    DEBUG_ME;
    lexer_t* lexer = lexer_create(filepath, source, selected_language);

    lex(lexer);

    lexer->destroy(lexer);

    return EXIT_SUCCESS;
}

int handle_file_command(const char *filename, language_map_t selected_language) {
    DEBUG_ME;
    if (!file_exists(filename)) {
        fprintf(stderr, "Error: File '%s' does not exist\n", filename);
        return EXIT_FAILURE;
    }

    char *source = file_reads_binary(filename, NULL);
    if (!source) {
        fprintf(stderr, "Error: Unable to open file '%s'\n", filename);
        return EXIT_FAILURE;
    }

    return run_code(source, filename, selected_language);
}

int handle_code_command(const char *source, language_map_t selected_language) {
    DEBUG_ME;
    return run_code(source, NULL, selected_language);
}

int handle_missing_arguments() {
    DEBUG_ME;
    fprintf(stderr, "Error: Missing arguments\n");
    print_help();
}

int handle_missing_language_value(char *value) {
    DEBUG_ME;
    fprintf(stderr, "Error: Unsupported language '%s'. Supported languages: en, fa.\n", value);
    return EXIT_FAILURE;
}

int process_language_flag(int argc, char **argv, language_map_t *selected_language) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--lang") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "fa") == 0) {
                    *selected_language = language_maps[1];
                } else if (strcmp(argv[i + 1], "en") == 0) {
                    *selected_language = language_maps[0];
                } else {
                    return handle_missing_language_value(argv[i + 1]);
                }
                i++;
            } else {
                fprintf(stderr, "Error: Missing language code after --lang\n");
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

int process_command(int argc, char **argv, language_map_t selected_language) {
    if (strcmp(argv[1], "help") == 0) {
        print_help();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "version") == 0) {
        print_version();
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "file") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing filename\n");
            return EXIT_FAILURE;
        }
        return handle_file_command(argv[2], selected_language);
    }

    if (strcmp(argv[1], "code") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing code\n");
            return EXIT_FAILURE;
        }
        return handle_code_command(argv[2], selected_language);
    }

    fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
    print_help();
    return EXIT_FAILURE;
}

int main(int argc, char **argv) {
    DEBUG_ME;

    if (argc < 2) {
        return handle_missing_arguments();
    }

    language_map_t selected_language = language_maps[0];

    int result = process_language_flag(argc, argv, &selected_language);
    if (result != EXIT_SUCCESS) {
        return result;
    }

    return process_command(argc, argv, selected_language);
}
