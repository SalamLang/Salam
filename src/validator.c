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

#include "generated-config/ast_layout_attribute_style_value.h"

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
    // value attribute for input tag
    if ((attribute->parent_node_type == AST_LAYOUT_TYPE_INPUT) &&
        (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT)) {
        attribute->final_key = string_strdup("value");
        return true;
    } else if (is_layout_node_a_single_tag(attribute->parent_node_type) &&
               attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return false;
    } else if (attribute->parent_node_type == AST_LAYOUT_TYPE_LABEL &&
               attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_FOR) {
        return true;
    } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return true;
    } else if (attribute->parent_node_type == AST_LAYOUT_TYPE_DIV &&
               attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_CONTENT) {
        return true;
    } else if (attribute->parent_node_type == AST_LAYOUT_TYPE_INCLUDE &&
               (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_SRC ||
                attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT)) {
        attribute->ignoreMe = true;
        return true;
    } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_REPEAT) {
        // TODO: add validation to only accepts positive integer
        attribute->ignoreMe = true;
        return true;
    } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_GROUP) {
        return true;
    } else if (is_style_attribute(attribute_key_type)) {
        attribute->isStyle = true;

        return true;
    } else if (is_attribute_type_a_style(attribute_key_type)) {
        return true;
    }

    // LAYOUT
    if (attribute->parent_node_type == AST_LAYOUT_TYPE_NONE) {
        if (is_attribute_type_in_array(attribute_key_type,
                                       valid_layout_attributes,
                                       valid_layout_attributes_length)) {
            return true;
        }
    }
    // BR, HR
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_BR ||
             attribute->parent_node_type == AST_LAYOUT_TYPE_HR) {
        // TODO: Maybe later?
        // return true;
        return false;
    }
    // RESPONSIVE
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_MEDIA) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MAX_HEIGHT,
            AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MIN_HEIGHT,
            AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MAX_WIDTH,
            AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MIN_WIDTH,
        };
        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            if (attribute_key_type ==
                    AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MAX_HEIGHT ||
                attribute_key_type ==
                    AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MIN_HEIGHT ||
                attribute_key_type ==
                    AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MAX_WIDTH ||
                attribute_key_type ==
                    AST_LAYOUT_ATTRIBUTE_TYPE_RESPONSIVE_MIN_WIDTH) {
                if (validate_style_value_size(NULL, NULL, attribute, NULL,
                                              NULL) == false) {
                    error_validator(
                        2,
                        "Invalid value for condition attribute '%s' in '%s' "
                        "element at line %zu column %zu!",
                        attribute->key,
                        ast_layout_node_type_to_enduser_name(
                            attribute->parent_node_type),
                        attribute->value_location.start_line,
                        attribute->value_location.start_column);

                    return false;
                }

                return true;
            }

            return true;
        }

        return false;
    }
    // FONT
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FONT) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
            AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
            AST_LAYOUT_ATTRIBUTE_TYPE_FONT_STYLE,
            AST_LAYOUT_ATTRIBUTE_TYPE_FONT_WEIGHT,
            AST_LAYOUT_ATTRIBUTE_TYPE_FONT_UNICODE_RANGE,
        };

        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            attribute->final_key = string_strdup(
                generator_code_layout_attribute_name(attribute_key_type));

            // name
            if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_NAME) {
                ast_value_t *value = attribute->values->data[0];

                if (value->type->kind == AST_TYPE_KIND_STRING &&
                    strlen(value->data.string_value) == 0) {
                    error_validator(2,
                                    "Invalid value for attribute '%s' in '%s' "
                                    "element at line %zu column %zu!",
                                    attribute->key,
                                    ast_layout_node_type_to_enduser_name(
                                        attribute->parent_node_type),
                                    attribute->value_location.start_line,
                                    attribute->value_location.start_column);

                    return false;
                }

                if (attribute->final_key != NULL) {
                    memory_destroy(attribute->final_key);
                }

                attribute->final_key = string_strdup("font-family");

                return true;
            }
            // src
            else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_SRC) {
                string_t *buffer = string_create(10);

                for (size_t i = 0; i < attribute->values->length; i++) {
                    ast_value_t *value = attribute->values->data[i];

                    if (value->type->kind == AST_TYPE_KIND_STRING) {
                        char *out_extension = NULL;

                        if (has_font_extension(value->data.string_value,
                                               &out_extension) &&
                            out_extension != NULL) {
                            string_append_str(buffer, "url('");
                            string_append_str(buffer, value->data.string_value);
                            string_append_char(buffer, '\'');
                            string_append_char(buffer, ')');

                            string_append_str(buffer, " format('");
                            string_append_str(buffer, out_extension);
                            string_append_char(buffer, '\'');
                            string_append_char(buffer, ')');
                        } else {
                            string_append_str(buffer, "local('");
                            string_append_str(buffer, value->data.string_value);
                            string_append_str(buffer, "')");
                        }

                        if (out_extension != NULL) {
                            memory_destroy(out_extension);
                        }

                        if (i < attribute->values->length - 1) {
                            string_append_char(buffer, ',');
                            string_append_char(buffer, ' ');
                        }
                    } else {
                        error_validator(2,
                                        "Invalid value for attribute '%s' in "
                                        "'%s' element at line %zu column %zu!",
                                        attribute->key,
                                        ast_layout_node_type_to_enduser_name(
                                            attribute->parent_node_type),
                                        attribute->value_location.start_line,
                                        attribute->value_location.start_column);

                        return false;
                    }
                }

                if (attribute->final_value != NULL) {
                    memory_destroy(attribute->final_value);
                }

                attribute->final_value = string_strdup(buffer->data);

                string_destroy(buffer);

                return true;
            } else if (attribute_key_type ==
                       AST_LAYOUT_ATTRIBUTE_TYPE_FONT_STYLE) {
                if (validate_style_value_string(
                        NULL, NULL, attribute,
                        ast_layout_allowed_style_list_font_style,
                        NULL) == false) {
                    error_validator(2,
                                    "Invalid value for attribute '%s' in '%s' "
                                    "element at line %zu column %zu!",
                                    attribute->key,
                                    ast_layout_node_type_to_enduser_name(
                                        attribute->parent_node_type),
                                    attribute->value_location.start_line,
                                    attribute->value_location.start_column);

                    return false;
                }

                return true;
            } else if (attribute_key_type ==
                       AST_LAYOUT_ATTRIBUTE_TYPE_FONT_WEIGHT) {
                if (validate_style_value_integer(
                        NULL, NULL, attribute,
                        ast_layout_allowed_style_list_font_weight,
                        NULL) == false) {
                    error_validator(2,
                                    "Invalid value for attribute '%s' in '%s' "
                                    "element at line %zu column %zu!",
                                    attribute->key,
                                    ast_layout_node_type_to_enduser_name(
                                        attribute->parent_node_type),
                                    attribute->value_location.start_line,
                                    attribute->value_location.start_column);

                    return false;
                }

                return true;
            } else if (attribute_key_type ==
                       AST_LAYOUT_ATTRIBUTE_STYLE_TYPE_FONT_DISPLAY) {
                if (validate_style_value_string(
                        NULL, NULL, attribute,
                        ast_layout_allowed_style_list_font_display,
                        NULL) == false) {
                    error_validator(2,
                                    "Invalid value for attribute '%s' in '%s' "
                                    "element at line %zu column %zu!",
                                    attribute->key,
                                    ast_layout_node_type_to_enduser_name(
                                        attribute->parent_node_type),
                                    attribute->value_location.start_line,
                                    attribute->value_location.start_column);

                    return false;
                }

                return true;
            } else if (attribute_key_type ==
                       AST_LAYOUT_ATTRIBUTE_TYPE_FONT_UNICODE_RANGE) {
                // TODO: We need to do more check layer later!
                return true;
            }

            return true;
        }
    }
    // TEXTAREA
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_TEXTAREA) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_PLACEHOLDER,
            AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
            AST_LAYOUT_ATTRIBUTE_TYPE_VALUE,  // Not needed
        };

        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            return true;
        }
    }
    // INPUT
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_INPUT) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_PLACEHOLDER,
            AST_LAYOUT_ATTRIBUTE_TYPE_NAME,
            AST_LAYOUT_ATTRIBUTE_TYPE_VALUE,
        };

        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            return true;
        }
    }
    // LINK
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_LINK) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
            AST_LAYOUT_ATTRIBUTE_TYPE_TYPE,
        };

        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            // rename type/نوع to target
            if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_TYPE) {
                if (attribute->final_key != NULL) {
                    memory_destroy(attribute->final_key);
                }

                attribute->final_key = string_strdup("target");

                return true;
                // rename src/منبع to href
            } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_SRC) {
                if (attribute->final_key != NULL) {
                    memory_destroy(attribute->final_key);
                }

                attribute->final_key = string_strdup("href");

                return true;
            }
        }
    }
    // SUBTITLE
    // else if (attribute->parent_node_type == AST_LAYOUT_TYPE_SUBTITLE) {
    //     ast_layout_attribute_type_t valid_attributes[] = {
    //         AST_LAYOUT_ATTRIBUTE_TYPE_TYPE,
    //     };

    //     size_t valid_attributes_length =
    //         sizeof(valid_attributes) / sizeof(valid_attributes[0]);

    //     if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
    //                                    valid_attributes_length)) {
    //         return true;
    //     }
    // }
    // FORM
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_FORM) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_TYPE,
            AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
        };

        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            if (attribute->final_key != NULL) {
                memory_destroy(attribute->final_key);
            }

            // rename type/نوع to method
            if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_TYPE) {
                attribute->final_key = string_strdup("method");
                // rename src/منبع to action
            } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_SRC) {
                attribute->final_key = string_strdup("action");
                // rename kind/روش to target
            } else if (attribute_key_type == AST_LAYOUT_ATTRIBUTE_TYPE_KIND) {
                attribute->final_key = string_strdup("target");
            }

            return true;
        }
    }
    // IMG
    else if (attribute->parent_node_type == AST_LAYOUT_TYPE_IMG) {
        ast_layout_attribute_type_t valid_attributes[] = {
            AST_LAYOUT_ATTRIBUTE_TYPE_SRC,
        };

        size_t valid_attributes_length =
            sizeof(valid_attributes) / sizeof(valid_attributes[0]);

        if (is_attribute_type_in_array(attribute_key_type, valid_attributes,
                                       valid_attributes_length)) {
            return true;
        }
    }

    return false;
}
