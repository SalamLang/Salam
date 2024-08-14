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

/**
 * 
 * @function is_attribute_type_in_array
 * @brief Check if the attribute type is in the array
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @params {ast_layout_attribute_type_t*} array - Array of attribute types
 * @params {size_t} array_size - Array size
 * @returns {bool} - True if the attribute type is in the array, false otherwise
 * 
 */
bool is_attribute_type_in_array(ast_layout_attribute_type_t type, ast_layout_attribute_type_t* array, size_t array_size)
{
    for (size_t i = 0; i < array_size; i++) {
        if (array[i] == type) {
            return true;
        }
    }
    
    return false;
}

/**
 * 
 * @function token_belongs_to_ast_layout_node
 * @brief Check if the token belongs to the AST layout node
 * @params {ast_layout_block_t*} block - AST layout block node
 * @params {ast_layout_attribute_type_t} attribute_key_type - Attribute key type
 * @returns {bool} - True if the token belongs to the AST layout node, false otherwise
 * 
 */
bool token_belongs_to_ast_layout_node(ast_layout_block_t* block, ast_layout_attribute_type_t attribute_key_type)
{
    if (block->parent_type != AST_NODE_TYPE_LAYOUT) {
        error(2, "Block parent type is not a layout node");
    }

    if (is_layout_node_a_single_tag(block->parent_node_type) && attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return false;
    }

    if (block->parent_node_type == AST_LAYOUT_NODE_TYPE_NONE) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
            AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
            AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
            AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
            AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
            AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
            AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
            AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
        };

        size_t valid_attributes_size = sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_size)) {
            return true;
        }
    }

    return false;
}

/**
 * 
 * @function is_layout_node_a_single_tag
 * @brief Check if the layout node is a single tag
 * @params {ast_layout_node_type_t} type - Layout node type
 * @returns {bool} - True if the layout node is a single tag, false otherwise
 * 
 */
bool is_layout_node_a_single_tag(ast_layout_node_type_t type)
{
    switch (type) {
        case AST_LAYOUT_NODE_TYPE_PARAGRAPH_RAW:
        case AST_LAYOUT_NODE_TYPE_INPUT:
        case AST_LAYOUT_NODE_TYPE_BR:
        case AST_LAYOUT_NODE_TYPE_HR:
            return true;

        default:
            return false;
    }
}

/**
 * 
 * @function validate_layout_mainbody
 * @brief Validate the layout mainbody
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 * 
 */
void validate_layout_mainbody(ast_layout_block_t* block)
{
    ast_layout_attribute_type_t valid_attributes[] = {
        AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,
        AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
        AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,
        AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
        AST_LAYOUT_ATTRIBUTE_TYPE_ICON,
        AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
        AST_LAYOUT_ATTRIBUTE_TYPE_DIR,
        AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
    };
    size_t valid_attributes_size = sizeof(valid_attributes) / sizeof(valid_attributes[0]);

    if (block->attributes != NULL) {
		if (block->attributes->data != NULL) {

            size_t attributes_capacity = block->attributes->capacity;

			for (size_t i = 0; i < attributes_capacity; i++) {
				hashmap_entry_t *entry = block->attributes->data[i];

				while (entry) {
                    char* attribute_key = entry->key;
                    ast_layout_attribute_type_t attribute_key_type = name_to_ast_layout_attribute_type(attribute_key);
                    ast_layout_attribute_t* attribute_value = entry->value;
                    
                    if (is_attribute_type_in_array(attribute_key_type, valid_attributes, valid_attributes_size)) {
                        attribute_value->ignoreMe = true;
                    }

        			entry = entry->next;
				}
			}
		}
    }
}
