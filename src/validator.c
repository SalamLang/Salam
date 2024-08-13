#include "validator.h"

/**
 * 
 * @function validate_layout_block
 * @brief Validate and modify the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 * 
 */
void validate_layout_block(ast_layout_block_t* block)
{
    hashmap_t attributes = cast(hashmap_t*, block->attributes);

    ast_layout_attribute_t* attribute_content = hashmap_get(attributes, "content");
    if (attribute_content != NULL) {
        if (attribute_content->values->size > 1) {
            unknown_scope(NULL, "layout block content");
        }
    }
}
