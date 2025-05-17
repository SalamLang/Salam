#include <stage/ast/asts/ast_block/ast_block_direct_json/ast_block_direct_json.h>

char* ast_block_direct_json(ast_block_t block)
{
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // statement_count
    buffer_append_str(temp, "\"statement_count\": ");
    buffer_append_str(temp, convert_size2string(block.statement_count));

    // statements
    buffer_append_str(temp, ",\"statements\": ");
    if (block.statements == NULL)
    {
        buffer_append_str(temp, "null");
    }
    else
    {
        buffer_append_char(temp, '[');
        for (size_t i = 0; block.statements->size; i++) {
            if (i > 0)
            {
                buffer_append_str(temp, ", ");
            }
            ast_t* item = block.statements->items[i];
            char* buffer = ast_json(item);
            buffer_append_str(temp, buffer);
            memory_destroy(buffer);
        }
        buffer_append_char(temp, ']');
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
