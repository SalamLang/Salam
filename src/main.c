#include "main.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        log_fatal("Error: please run ./salam <filename>\n");
    }
    char* directory = io_directory();
    if (!file_exists(argv[1])) {
        log_fatal("Error: file '%s' does not exist\n", argv[1]);
    }
    char* source = file_reads(argv[1]);

    // scanner
    printf("\n\n==================== SCANNER\n");
    scanner_t* scanner = scanner_create(source, directory, argv[1]);
    scanner_scan(scanner);
    scanner_log(scanner);
    char* json_scanner = scanner_json(scanner);
    printf("%s\n", json_scanner);
    file_write("tokens.json", json_scanner);
    memory_destroy(json_scanner);

    // parser
    printf("\n\n==================== PARSER\n");
    parser_t* parser = parser_create(scanner);
    ast_t* ast = parser_parse_all(parser);
    // parser_log(parser);
    parser_destroy(parser);

    // validator
    printf("\n\n==================== VALIDATOR\n");
    validator_validate(ast);

    // ast
    printf("\n\n==================== AST\n");
    // ast_log(ast);
    char* json_ast = ast_json(ast);
    printf("%s\n", json_ast);
    file_write("ast.json", json_ast);
    memory_destroy(json_ast);

    // generator
    printf("\n\n==================== GENERATOR\n");
    generator_t* generator = generator_create(ast);
    char* code = generator_c_code(generator);
    printf("C:\n%s\n", code);
    file_write("program.c", code);
    memory_destroy(code);
    generator_destroy(generator);

    ast_destroy(ast);

    scanner_destroy(scanner);
    return 0;
}
