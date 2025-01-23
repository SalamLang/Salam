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

#include "lexer.h"

void print_help() {
    DEBUG_ME;
    printf("Usage:\n");
    printf("  ./s file <filename>      Run code from a file\n");
    printf("  ./s code <code inline>   Run inline code\n");
    printf("  ./s --help               Show this help message\n");
    printf("  ./s --version            Show the program version\n");
}

void print_version() {
    DEBUG_ME;
    printf("Version: %s\n", SALAM_VERSION);
}

int handle_file_command(const char *filename) {
    DEBUG_ME;
    if (!file_exists(filename)) {
        fprintf(stderr, "Error: File '%s' does not exist\n", filename);
        return EXIT_FAILURE;
    }

    char *content = file_reads_binary(filename, NULL);
    if (!content) {
        fprintf(stderr, "Error: Unable to open file '%s'\n", filename);
        return EXIT_FAILURE;
    }

    printf("%s", content);

    return EXIT_SUCCESS;
}

int handle_code_command(const char *code) {
    DEBUG_ME;
    printf("Running inline code: %s\n", code);
    return EXIT_SUCCESS;
}

void handle_missing_arguments() {
    DEBUG_ME;
    fprintf(stderr, "Error: Missing arguments\n");
    print_help();
}

int main(int argc, char **argv) {
    DEBUG_ME;
    if (argc < 2) {
        handle_missing_arguments();
        return EXIT_FAILURE;
    }

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
        return handle_file_command(argv[2]);
    }

    if (strcmp(argv[1], "code") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: Missing code\n");
            return EXIT_FAILURE;
        }
        return handle_code_command(argv[2]);
    }

    fprintf(stderr, "Error: Unknown command '%s'\n", argv[1]);
    print_help();
    return EXIT_FAILURE;
}
