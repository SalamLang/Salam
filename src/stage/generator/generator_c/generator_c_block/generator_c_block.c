#include "generator_c_block.h"

buffer_t* generator_c_block(generator_t* generator, ast_t* ast)
{
    buffer_t* temp = buffer_create(124);

    ast_block_t block = ast->raw.block_value;

    buffer_append_str(temp, "{\n");

    buffer_append_str(temp, "\t// TODO\n");
    for (size_t i = 0; i < block.statements->size; i++) {
        ast_t* node = block.statements->items[i];
        buffer_t* node_temp = generator_c_node(generator, node);
        buffer_append(temp, node_temp);
        buffer_destroy(node_temp);
    }

    buffer_append_str(temp, "}\n");

    return temp;
}
