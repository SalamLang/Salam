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

#include "generated-config/ast_layout_attribute_style_value_extern.h"
#include "generated-config/ast_layout_attribute_value.h"
#include "generated-config/ast_layout_type_attributes_values_extern.h"

/**
 *
 * @function is_layout_node_a_single_tag
 * @brief Check if the layout node is a single tag
 * @params {ast_layout_node_type_t} type - Layout node type
 * @returns {bool} - True if the layout node is a single tag, false
 * otherwise
 *
 */
bool is_layout_node_a_single_tag(ast_layout_node_type_t type) {
    DEBUG_ME;
    switch (type) {
#undef ADD_LAYOUT_TYPE
#undef ADD_LAYOUT_TYPE_HIDE
#undef ADD_LAYOUT_TYPE_REPEAT

#define ADD_LAYOUT_TYPE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, ENDUSER_NAME, \
                        IS_SINGLE)                                            \
    case TYPE:                                                                \
        return IS_SINGLE;

#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                             ENDUSER_NAME, IS_SINGLE)                \
    case TYPE:                                                       \
        return IS_SINGLE;

#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                               ENDUSER_NAME, IS_SINGLE)

#include "generated-config/ast_layout_type.h"
    }

    return false;
}

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
                                attribute_key_type, valid_attributes_layout,
                                valid_attributes_layout_length)) {
                            attribute_value->ignoreMe = true;
                            attribute_value->belongsToLayout = true;
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
 * @function is_attribute_type_a
 * @brief Check if the attribute type is
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is, false otherwise
 *
 */
bool is_attribute_type_a(ast_layout_attribute_type_t type) {
    DEBUG_ME;
    switch (type) {
#undef ADD_LAYOUT_ATTRIBUTE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_TYPE_HIDE
#undef ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, \
                                        GENERATED_NAME, FILTER,               \
                                        ALLOWED_VALUES, SUBTAGS, VALUE_HANDLER)              \
    case TYPE:

#define ADD_LAYOUT_ATTRIBUTE_TYPE_HIDE(TYPE, NAME, NAME_LOWER,       \
                                             ENDUSER_NAME, GENERATED_NAME, \
                                             FILTER, ALLOWED_VALUES, SUBTAGS, VALUE_HANDLER)

#define ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT(                   \
    TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, \
    ALLOWED_VALUES, SUBTAGS, VALUE_HANDLER)

#include "generated-config/ast_layout_attribute_type.h"
        return true;

        default:
            return false;
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
         attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT)) {
        attribute->ignoreMe = true;

        return true;
    } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT) {
        // TODO: add validation to only accepts positive integer
        attribute->ignoreMe = true;

        return true;
    } else if (is_attribute_style_type_a_style(attribute_key_type)) {
        attribute->isStyle = true;

        return true;
    } else if (is_attribute_type_a(attribute_key_type)) {
        return true;
    }

    if (false) {
    }
#include "generated-config/ast_layout_type_attributes.h"

    if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return is_layout_node_a_single_tag(attribute->parent_node_type) ? false
                                                                        : true;
    }

    return false;
}

/**
 *
 * @function validate_value
 * @brief Validate the value
 * @params {hashmap_t*} attributes - Attributes
 * @params {hashmap_t*} new_attributes - New Attributes
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the value is valid, false otherwise
 *
 */
bool validate_value(hashmap_t *attributes, hashmap_t *new_attributes,
                    ast_layout_attribute_t *attribute) {
    DEBUG_ME;
    ast_value_t *first = attribute->values->data[0];

    if (attribute->values->length < 1) {
        error_validator(
            2, "Attribute value is missing in '%s' element",
            ast_layout_node_type_to_enduser_name(attribute->parent_node_type));

        return false;
    } else if (first->type->kind == AST_TYPE_KIND_STRING &&
               strlen(first->data.string_value) == 0) {
        error_validator(
            2,
            "Empty value for '%s' attribute in '%s' element is "
            "not allowed at "
            "line %zu column %zu!",
            attribute->key,
            ast_layout_node_type_to_enduser_name(attribute->parent_node_type),
            attribute->value_location.start_line,
            attribute->value_location.start_column);

        return false;
    }

    switch (attribute->type) {
#undef ADD_LAYOUT_ATTRIBUTE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_TYPE_HIDE
#undef ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME,        \
                                  GENERATED_NAME, FILTER, ALLOWED_VALUES,      \
                                  SUBTAGS, VALUE_HANDLER)                                     \
    case TYPE: {                                                               \
        attribute->final_key = string_strdup(GENERATED_NAME);                  \
        const ast_layout_attribute_pair_t *values = ALLOWED_VALUES;            \
                                                                               \
        if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_COLOR) {                     \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_color(                                 \
                attributes, new_attributes, attribute,                         \
                ast_layout_allowed_style_color, values);                       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_STRING) {             \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_string(attributes, new_attributes,     \
                                               attribute, NULL, values);       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_STRING_ANY) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_STRINGS_ANY) {        \
            attribute->final_value =                                           \
                array_value_stringify(attribute->values, ",");                 \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_SIZE) {               \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_size(attributes, new_attributes,       \
                                             attribute, NULL, values);         \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_SIZE_COLOR) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_size_color(attributes, new_attributes, \
                                                   attribute, NULL, values);   \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_SIZES) {              \
            return validate_style_value_sizes(attributes, new_attributes,      \
                                              attribute, NULL, values, false); \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_SIZES124) {           \
            return validate_style_value_sizes(attributes, new_attributes,      \
                                              attribute, NULL, values, true);  \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_COLORS) {             \
            return validate_style_value_colors(attributes, new_attributes,     \
                                               attribute, NULL, values);       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_PERCENTAGE) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_percentage(attributes, new_attributes, \
                                                   attribute, NULL, values);   \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_SIZES_COLORS) {       \
            return validate_style_value_sizes_colors(                          \
                attributes, new_attributes, attribute, NULL, values);          \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_INTEGER) {            \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_integer(attributes, new_attributes,    \
                                                attribute, NULL, values);      \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_FLOAT) {              \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_float(attributes, new_attributes,      \
                                              attribute, NULL, values);        \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_NUMBER) {             \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_number(attributes, new_attributes,     \
                                               attribute, NULL, values);       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_POSITIVE_INTEGER) {   \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_integer(attributes, new_attributes,    \
                                                attribute, NULL, values);      \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_POSITIVE_FLOAT) {     \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_float(attributes, new_attributes,      \
                                              attribute, NULL, values);        \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_POSITIVE_NUMBER) {    \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_number(attributes, new_attributes,     \
                                               attribute, NULL, values);       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_NEGATIVE_INTEGER) {   \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_integer(attributes, new_attributes,    \
                                                attribute, NULL, values);      \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_NEGATIVE_FLOAT) {     \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_float(attributes, new_attributes,      \
                                              attribute, NULL, values);        \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_NEGATIVE_NUMBER) {    \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_number(attributes, new_attributes,     \
                                               attribute, NULL, values);       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_TIME) {               \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_PIXEL) {              \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUT_ATTRIBUTE_FILTER_GROUP) {              \
            return true;                                                       \
        } else if (FILTER ==                                                   \
                   AST_LAYOUT_ATTRIBUTE_FILTER_POSITIVE_PERCENTAGE) {          \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER ==                                                   \
                   AST_LAYOUT_ATTRIBUTE_FILTER_NEGATIVE_PERCENTAGE) {          \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        }                                                                      \
    }

#define ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, \
                                         GENERATED_NAME, FILTER,               \
                                         ALLOWED_VALUES, SUBTAGS, VALUE_HANDLER)

#define ADD_LAYOUT_ATTRIBUTE_TYPE_HIDE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME,   \
                                       GENERATED_NAME, FILTER, ALLOWED_VALUES, \
                                       SUBTAGS, VALUE_HANDLER)

#include "generated-config/ast_layout_attribute_type.h"

        default:
            return false;
    }

    // return false;
}
