/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 *
 * Welcome to the Salam Programming Language! Salam is a modern, efficient,
 * and developer-friendly language created to inspire and empower programmers
 * of all ages. With its unique features and Persian roots, Salam aims to make
 * programming more accessible and enjoyable for everyone.
 *
 * This file is part of the core implementation of Salam, including its runtime
 * and compiler components. By contributing to or using this codebase, you are
 * part of a growing community dedicated to innovation and inclusivity in
 * programming.
 *
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 *
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

#include "validator.h"

#include "validator_style.h"

// #include "generated-config/ast_layout_attribute_value.h"
// #include "generated-config/ast_layout_attribute_style_value.h"

extern const ast_layout_attribute_style_pair_t
    ast_layout_allowed_style_list_font_display[];
extern const ast_layout_attribute_style_pair_t
    ast_layout_allowed_style_list_font_style[];
extern const ast_layout_attribute_style_pair_t
    ast_layout_allowed_style_list_font_weight[];

/**
 *
 * @var valid_layout_attributes
 * @brief Valid layout attributes
 * @type {ast_layout_attribute_type_t[]}
 */
ast_layout_attribute_type_t valid_layout_attributes[] = {
    AST_LAYOUT_ATTRIBUTE_TYPE_TITLE,    AST_LAYOUT_ATTRIBUTE_TYPE_DESCRIPTION,
    AST_LAYOUT_ATTRIBUTE_TYPE_AUTHOR,   AST_LAYOUT_ATTRIBUTE_TYPE_KEYWORDS,
    AST_LAYOUT_ATTRIBUTE_TYPE_ICON,     AST_LAYOUT_ATTRIBUTE_TYPE_CHARSET,
    AST_LAYOUT_ATTRIBUTE_TYPE_DIR,      AST_LAYOUT_ATTRIBUTE_TYPE_LANG,
    AST_LAYOUT_ATTRIBUTE_TYPE_VIEWPORT, AST_LAYOUT_ATTRIBUTE_TYPE_REFRESH,
};

/**
 *
 * @var valid_layout_attributes_length
 * @brief Valid layout attributes length
 * @type {size_t}
 */
size_t valid_layout_attributes_length =
    sizeof(valid_layout_attributes) / sizeof(valid_layout_attributes[0]);

/**
 *
 * @function validate_layout_block
 * @brief Validate and modify the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void validate_layout_block(ast_layout_block_t *block) {
    DEBUG_ME;
    hashmap_layout_attribute_t *attributes = block->attributes;

    ast_layout_attribute_t *attribute_content =
        hashmap_get(attributes, "content");

    if (attribute_content != NULL) {
        attribute_content->isContent = true;

        array_value_t *values = attribute_content->values;

        if (values->length > 0) {
            char *content = array_value_stringify(values, ", ");

            if (content != NULL) {
                if (strlen(content) > 0) {
                    block->text_content = content;
                } else {
                    if (content != NULL) {
                        memory_destroy(content);
                    }
                }
            }
        }
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
bool is_attribute_type_in_array(ast_layout_attribute_type_t type,
                                ast_layout_attribute_type_t *array,
                                size_t array_size) {
    DEBUG_ME;
    for (size_t i = 0; i < array_size; i++) {
        if (array[i] == type) {
            return true;
        }
    }

    return false;
}

/**
 *
 * @function validate_layout_mainbody
 * @brief Validate the layout mainbody
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 *
 */
void validate_layout_mainbody(ast_layout_block_t *block) {
    DEBUG_ME;
    if (block != NULL) {
        if (block->attributes != NULL) {
            if (block->attributes->data != NULL) {
                size_t attributes_capacity = block->attributes->capacity;

                for (size_t i = 0; i < attributes_capacity; i++) {
                    hashmap_entry_t *entry = block->attributes->data[i];

                    while (entry) {
                        char *attribute_key = entry->key;
                        ast_layout_attribute_type_t attribute_key_type =
                            name_to_ast_layout_attribute_type(attribute_key);
                        ast_layout_attribute_t *attribute_value = entry->value;

                        if (is_attribute_type_in_array(
                                attribute_key_type, valid_layout_attributes,
                                valid_layout_attributes_length)) {
                            attribute_value->ignoreMe = true;
                        }

                        entry = cast(hashmap_entry_t *, entry->next);
                    }
                }
            }
        }
    }
}

/**
 *
 * @function token_belongs_to_ast_layout_node
 * @brief Check if the token belongs to the AST layout node
 * @params {ast_layout_attribute_type_t} attribute_key_type - Attribute key type
 * @params {ast_layout_attribute_t*} attribute - AST layout attribute
 * @returns {bool} - True if the token belongs to the AST layout node, false
 * otherwise
 *
 */
bool token_belongs_to_ast_layout_node(
    ast_layout_attribute_type_t attribute_key_type,
    ast_layout_attribute_t *attribute) {
    DEBUG_ME;

    if (attribute->parent_node_type == AST_LAYOUT_TYPE_INCLUDE &&
                   (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_SRC ||
                    attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT))
                    {
            attribute->ignoreMe = true;

            return true;
    } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT) {
        // TODO: add validation to only accepts positive integer
        attribute->ignoreMe = true;

        return true;
    } else if (is_style_attribute(attribute_key_type)) {
        attribute->isStyle = true;

        return true;
    } else if (is_attribute_type_a_style(attribute_key_type)) {
        return true;
    }

    if (false) {
    }
#include "generated-config/ast_layout_type_attributes.h"
    
    if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return is_layout_node_a_single_tag(attribute->parent_node_type) ? false : true;
    }

    return false;
}
