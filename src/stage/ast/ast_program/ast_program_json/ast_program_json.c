#include <stage/ast/ast_program/ast_program_json/ast_program_json.h>

char* ast_program_json(ast_program_t* block)
{
    buffer_t* temp = buffer_create(24);

    // TODO

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
