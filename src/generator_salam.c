#include "generator_salam.h"

#define IDENT(SIZE) generator_salam_ident(salam, SIZE);

/**
 *
 * @function generator_salam
 * @brief Generate the Salam code (cleaned and formatted)
 * @params {ast_t*} ast - AST
 * @returns {string_t*}
 *
 */
string_t* generator_salam(ast_t* ast) {
    DEBUG_ME;
    string_t* salam = string_create(1024);

    generator_salam_layout(salam, ast->layout);

    return salam;
}

/**
 *
 * @function generator_salam_layout
 * @brief Generate the Salam code for the layout
 * @params {ast_layout_t*} layout - Layout
 * @returns {void}
 *
 */
void generator_salam_layout(string_t* salam, ast_layout_t* layout) {
    DEBUG_ME;
    generator_salam_layout_block(salam, layout->block);
}

/**
 *
 * @function generator_salam_layout_block
 * @brief Generate the Salam code for the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void generator_salam_layout_block(string_t* salam, ast_layout_block_t* block) {
    DEBUG_ME;
    // char* block_type_name = ast_block_type_name(block->type);

    char* block_node_name =
        ast_layout_node_type_to_enduser_name(block->parent_node_type);

    IDENT(0);
    string_append_str(salam, block_node_name);
    string_append_str(salam, ":\n");

    IDENT(0);
    string_append_str(salam, "تمام\n");
}

/**
 *
 * @function generator_salam_ident
 * @brief Generate the Salam code for the indentation
 * @params {string_t*} buffer - Buffer
 * @params {size_t} ident - Identation
 * @returns {void}
 *
 */
void generator_salam_ident(string_t* buffer, size_t ident) {
    DEBUG_ME;
    if (ident == 0) {
        return;
    }

    for (size_t i = 0; i < ident; i++) {
        // string_append_str(buffer, "  ");
        // string_append_str(buffer, "    ");
        // string_append_str(buffer, "      ");
        // string_append_str(buffer, "        ");
        // string_append_str(buffer, "\t\t");
        string_append_char(buffer, '\t');
    }
}
