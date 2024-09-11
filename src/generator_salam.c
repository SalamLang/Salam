#include "generator_salam.h"

#define IDENT(SIZE) generator_salam_ident(salam, SIZE);

size_t generator_salam_ident_size = 0;

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

    IDENT(generator_salam_ident_size);
    string_append_str(salam, block_node_name);
    string_append_str(salam, ":\n");

    generator_salam_ident_size++;

    generator_salam_layout_attributes(salam, block->attributes);

    generator_salam_layout_styles(salam, block->styles);

    generator_salam_layout_states(salam, block->states);

    if (block->children != NULL && block->children->length > 0) {
        for (size_t i = 0; i < block->children->length; i++) {
            ast_layout_node_t* node = array_get(block->children, i);
            generator_salam_layout_node(salam, node);
        }
    }

    generator_salam_ident_size--;

    IDENT(generator_salam_ident_size);
    string_append_str(salam, "تمام\n");
}

/**
 *
 * @function generator_salam_layout_node
 * @brief Generate the Salam code for the layout node
 * @params {string_t*} salam - Buffer
 * @params {ast_layout_node_t*} node - Layout node
 * @returns {void}
 *
 */
void generator_salam_layout_node(string_t* salam, ast_layout_node_t* node) {
    DEBUG_ME;
    char* node_name = ast_layout_node_type_to_enduser_name(node->type);

    IDENT(generator_salam_ident_size);
    string_append_str(salam, node_name);
    string_append_str(salam, ":\n");

    generator_salam_ident_size++;

    generator_salam_layout_attributes(salam, node->block->attributes);

    generator_salam_layout_styles(salam, node->block->styles);

    generator_salam_layout_states(salam, node->block->states);

    if (node->block->children != NULL && node->block->children->length > 0) {
        for (size_t i = 0; i < node->block->children->length; i++) {
            ast_layout_node_t* child = array_get(node->block->children, i);
            generator_salam_layout_node(salam, child);
        }
    }

    generator_salam_ident_size--;

    IDENT(generator_salam_ident_size);
    string_append_str(salam, "تمام\n");
}

/**
 *
 * @function generator_salam_layout_attribute
 * @brief Generate the Salam code for the attribute
 * @params {string_t*} salam - Buffer
 * @params {ast_layout_attribute_t*} attribute - Attribute
 * @returns {void}
 *
 */
void generator_salam_layout_attribute(string_t* salam,
                                      ast_layout_attribute_t* attribute) {
    DEBUG_ME;
    IDENT(generator_salam_ident_size);

    string_append_str(salam, attribute->key);
    string_append_str(salam, ": ");
    char* value = array_value_stringify(attribute->values, ", ");
    string_append_str(salam, value);
    memory_destroy(value);
    string_append_str(salam, "\n");
}

/**
 *
 * @function generator_salam_layout_attributes
 * @brief Generate the Salam code for the attributes
 * @params {string_t*} salam - Buffer
 * @params {hashmap_t*} attributes - Attributes
 * @returns {void}
 *
 */
void generator_salam_layout_attributes(string_t* salam, hashmap_t* attributes) {
    DEBUG_ME;
    if (attributes != NULL) {
        size_t attribute_capacity = attributes->capacity;

        for (size_t i = 0; i < attribute_capacity; i++) {
            hashmap_entry_t* entry = attributes->data[i];

            while (entry) {
                ast_layout_attribute_t* attribute =
                    cast(ast_layout_attribute_t*, entry->value);

                if (attribute->isStyle == true
                    // ||
                    // attribute->isContent == true ||
                    // attribute->ignoreMe == true
                ) {
                } else {
                    generator_salam_layout_attribute(salam, attribute);
                }

                entry = cast(hashmap_entry_t*, entry->next);
            }
        }
    }
}

/**
 *
 * @function generator_salam_layout_attributes_styles
 * @brief Generate the Salam code for the attributes styles
 * @params {string_t*} salam - Buffer
 * @params {hashmap_t*} attributes - Attributes styles
 * @returns {void}
 *
 */
void generator_salam_layout_attributes_styles(string_t* salam,
                                              hashmap_t* attributes) {
    DEBUG_ME;
    if (attributes != NULL) {
        size_t attribute_capacity = attributes->capacity;

        for (size_t i = 0; i < attribute_capacity; i++) {
            hashmap_entry_t* entry = attributes->data[i];

            while (entry) {
                ast_layout_attribute_t* attribute =
                    cast(ast_layout_attribute_t*, entry->value);

                if (attribute->isStyle == false || attribute->isContent == true
                    // ||
                    // attribute->ignoreMe == true
                ) {
                } else {
                    generator_salam_layout_attribute(salam, attribute);
                }

                entry = cast(hashmap_entry_t*, entry->next);
            }
        }
    }
}

/**
 *
 * @function generator_salam_layout_styles
 * @brief Generate the Salam code for the styles
 * @params {string_t*} salam - Buffer
 * @params {ast_layout_style_state_t*} styles - Styles
 * @returns {void}
 *
 */
void generator_salam_layout_styles(string_t* salam,
                                   ast_layout_style_state_t* styles) {
    DEBUG_ME;
    generator_salam_layout_attributes_styles(salam, styles->normal);

    generator_salam_layout_attributes_styles(salam, styles->new);
}

/**
 *
 * @function generator_salam_layout_states
 * @brief Generate the Salam code for the states
 * @params {string_t*} salam - Buffer
 * @params {hashmap_t*} states - States
 * @returns {void}
 *
 */
void generator_salam_layout_states(string_t* salam, hashmap_t* states) {
    DEBUG_ME;
    // TODO
}

/**
 *
 * @function generator_salam_ident
 * @brief Generate the Salam code for the indentation
 * @params {string_t*} salam - Buffer
 * @params {size_t} ident - Indentation
 * @returns {void}
 *
 */
void generator_salam_ident(string_t* salam, size_t ident) {
    DEBUG_ME;
    if (ident == 0) {
        return;
    }

    for (size_t i = 0; i < ident; i++) {
        // string_append_str(salam, "  ");
        // string_append_str(salam, "    ");
        // string_append_str(salam, "      ");
        // string_append_str(salam, "        ");
        // string_append_str(salam, "\t\t");
        string_append_char(salam, '\t');
    }
}
