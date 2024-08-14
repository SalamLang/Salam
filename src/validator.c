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
    hashmap_t* attributes = cast(hashmap_t*, block->attributes);

    ast_layout_attribute_t* attribute_content = hashmap_get(attributes, "content");
    if (attribute_content != NULL) {
        attribute_content->isContent = true;

        array_t* values = cast(array_t*, attribute_content->values);
        values->print(values);
        char* content = array_string_token(values, ", ");

        if (values->length > 0 && strlen(content) > 0) {
            printf("content: %s\n", content);
            block->text_content = content;
        }
        else {
            if (content != NULL) memory_destroy(content);
        }

        // ast_layout_node_t* text = ast_layout_node_create(AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW);
        // hashmap_put(cast(hashmap_t*, text->block->attributes), "content", values);
        // array_push(block->children, text);
    }
}
