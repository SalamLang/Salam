#include <stage/ast/ast_block/ast_block_json/ast_block_json.h>

char* ast_block_json(ast_block_t* block)
{
    buffer_t* temp = buffer_create(24);

    // TODO

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
