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

#include "validator_style.h"

#include "generated-config/ast_layout_attribute_value.h"
#include "generated-config/ast_layout_attribute_style_value.h"

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
                        IS_MOTHER)                                            \
    case TYPE:                                                                \
        return IS_MOTHER;

#define ADD_LAYOUT_TYPE_HIDE(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                             ENDUSER_NAME, IS_MOTHER)                \
    case TYPE:                                                       \
        return IS_MOTHER;

#define ADD_LAYOUT_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, GENERATED_NAME, \
                               ENDUSER_NAME, IS_MOTHER)

#include "generated-config/ast_layout_type.h"
    }

    return false;
}

/**
 *
 * @function has_font_extension
 * @brief Check if the value has a font extension
 * @params {char*} value - Value
 * @params {char**} out_extension - Output extension
 * @returns {bool} - True if the value has a font extension, false otherwise
 *
 */
bool has_font_extension(char *value, char **out_extension) {
    DEBUG_ME;
    if (value == NULL) {
        return false;
    }

    size_t len = strlen(value);
    if (len == 0) {
        return false;
    }

    // Source: https://www.w3.org/TR/css-fonts-4/
    const char *prefixes[] = {
        ".otc", ".ttc",  ".eot",  ".ttf",   ".otf",
        ".svg", ".svgz", ".woff", ".woff2",
    };
    const char *names[] = {
        "collection", "collection", "embedded-opentype", "opentype", "opentype",
        "svg",        "svg",        "truetype",          "woff",     "woff2",
    };

    size_t num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);

    for (size_t i = 0; i < num_prefixes; i++) {
        size_t prefix_len = strlen(prefixes[i]);

        if (len - prefix_len != 0 &&
            my_strcasecmp(value + len - prefix_len, prefixes[i]) == 0) {
            if (out_extension != NULL) {
                *out_extension = string_strdup(names[i]);
            }

            return true;
        }
    }

    return false;
}

/**
 *
 * @function has_css_size_prefix
 * @brief Check if the CSS value has a size prefix
 * @params {char*} css_value - CSS value
 * @params {char**} css_output_value - CSS output value
 * @returns {bool} - True if the CSS value has a size prefix, false otherwise
 *
 */
bool has_css_size_prefix(char *css_value, char **css_output_value) {
    DEBUG_ME;
    const char *prefixes[] = {"px",
                              "pixel",
                              "pixels",
                              "em",
                              "em",
                              "rem",
                              "vw",
                              "viewport width",
                              "viewport width",
                              "vh",
                              "viewport height",
                              "viewport height",
                              "%",
                              "cm",
                              "centimeter",
                              "centimeters",
                              "mm",
                              "millimeter",
                              "millimeters",
                              "in",
                              "inch",
                              "inches",
                              "pt",
                              "point",
                              "points",
                              "pc",
                              "pica",
                              "picas",
                              "ex",
                              "ch",
                              "vmin",
                              "viewport minimum",
                              "vmax",
                              "viewport maximum"};

    const char *persian_prefixes[] = {"پیکسل",     "پیکسل‌ها",
                                      "پیکسلها",   "ای ام",
                                      "ایم",       "رایم",
                                      "ویو ویدث",  "ویو ویدت",
                                      "ویویدث",    "ویو هایت",
                                      "ویو هایت",  "وی هایت",
                                      "درصد",      "سانتیمتر",
                                      "سانتی متر", "سانت",
                                      "میلیمتر",   "میلی متر",
                                      "میلیم",     "اینچ",
                                      "اینچ",      "اینچ",
                                      "پوینت",     "پوینت‌ها",
                                      "پوینتها",   "پیکا",
                                      "پیکاها",    "پیکاها",
                                      "اکس",       "سی اچ",
                                      "وی مین",    "وی مینیمم",
                                      "وی مکس",    "وی ماکسیمم"};

    const char *generated_prefixes[] = {
        "px", "px", "px", "em",   "em",   "rem",  "vw",   "vw", "vw",
        "vh", "vh", "vh", "%",    "cm",   "cm",   "cm",   "mm", "mm",
        "mm", "in", "in", "in",   "pt",   "pt",   "pt",   "pc", "pc",
        "pc", "ex", "ch", "vmin", "vmin", "vmax", "vmax",
    };

    size_t num_prefixes = sizeof(prefixes) / sizeof(prefixes[0]);
    size_t num_persian_prefixes =
        sizeof(persian_prefixes) / sizeof(persian_prefixes[0]);

    size_t len = strlen(css_value);
    if (len == 0) {
        *css_output_value = NULL;

        return false;
    }

    string_t *buffer = string_create(len + 1);

    size_t i = 0;

    if (css_value[i] == '-' || css_value[i] == '+') {
        if (css_value[i] == '-') {
            string_append_char(buffer, css_value[i]);
        }

        i++;
    }

    if (!isdigit(css_value[i])) {
        *css_output_value = NULL;

        string_destroy(buffer);

        return false;
    }

    bool decimal_point_found = false;
    while (i < len && (isdigit(css_value[i]) ||
                       (css_value[i] == '.' && !decimal_point_found))) {
        if (css_value[i] == '.') {
            string_append_char(buffer, css_value[i]);

            decimal_point_found = true;
        } else {
            string_append_char(buffer, css_value[i]);
        }

        i++;
    }

    while (i < len && isspace(css_value[i])) i++;

    for (size_t j = 0; j < num_prefixes; j++) {
        size_t prefix_len = strlen(prefixes[j]);

        if (len - i == prefix_len &&
            strncmp(css_value + i, prefixes[j], prefix_len) == 0) {
            string_append_str(buffer, generated_prefixes[j]);

            *css_output_value = string_strdup(buffer->data);

            string_destroy(buffer);

            return true;
        }
    }

    for (size_t j = 0; j < num_persian_prefixes; j++) {
        size_t prefix_len = strlen(persian_prefixes[j]);

        if (len - i == prefix_len &&
            strncmp(css_value + i, persian_prefixes[j], prefix_len) == 0) {
            string_append_str(buffer, generated_prefixes[j]);

            *css_output_value = string_strdup(buffer->data);

            string_destroy(buffer);

            return true;
        }
    }

    *css_output_value = NULL;

    string_destroy(buffer);

    return false;
}

/**
 *
 * @function normalise_css_size
 * @brief Normalise the CSS size
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - Normalised CSS size
 *
 */
char *normalise_css_size(char *attribute_value) {
    DEBUG_ME;
    if (!string_is_number(attribute_value) == true) {
        return string_strdup(attribute_value);
    }

    size_t value_length = strlen(attribute_value) + 3;

    char *res = memory_allocate(value_length * sizeof(char));

    snprintf(res, value_length, "%spx", attribute_value);

    return res;
}

/**
 *
 * @function attribute_css_size_value
 * @brief Convert the attribute value to a CSS size value
 * @params {char*} attribute_value - Attribute value
 * @returns {char*} - CSS size value
 *
 */
char *attribute_css_size_value(char *attribute_value) {
    DEBUG_ME;
    int attribute_value_length = strlen(attribute_value) + 5;  // mb2strlen

    char *res = memory_allocate(attribute_value_length * sizeof(char));

    strcpy(res, attribute_value);

    if (string_is_number(attribute_value) == true) {
        strcat(res, "px");
    }

    return res;
}

/**
 *
 * @function is_style_attribute
 * @brief Check if the attribute type is a CSS attribute
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 *
 */
bool is_style_attribute(ast_layout_attribute_type_t type) {
    DEBUG_ME;
    switch (type) {
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, \
                                        GENERATED_NAME, FILTER,               \
                                        ALLOWED_VALUES, SUBTAGS)              \
    case TYPE:

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER,       \
                                             ENDUSER_NAME, GENERATED_NAME, \
                                             FILTER, ALLOWED_VALUES, SUBTAGS)

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT(                   \
    TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, \
    ALLOWED_VALUES, SUBTAGS)

#include "generated-config/ast_layout_attribute_style_type.h"
        return true;

        default:
            return false;
    }
}

/**
 *
 * @function is_attribute_type_a_style
 * @brief Check if the attribute type is a style
 * @params {ast_layout_attribute_type_t} type - Attribute type
 * @returns {bool} - True if the attribute type is a style, false otherwise
 *
 */
bool is_attribute_type_a_style(ast_layout_attribute_type_t type) {
    DEBUG_ME;
    switch (type) {
        case AST_LAYOUT_ATTRIBUTE_TYPE_CLASS:
        case AST_LAYOUT_ATTRIBUTE_TYPE_ID:
            return true;

        default:
            return false;
    }
}

/**
 *
 * @function validate_style_value_string
 * @brief Validate the style value string
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_string(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }

    if (attribute->values == NULL || attribute->values->data == NULL ||
        attribute->values->data[0] == NULL) {
        return false;
    }

    ast_value_t *first = attribute->values->data[0];

    if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }
                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;

            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }
                i++;
            }
        }
    }

    return false;
}

/**
 *
 * @function validate_style_value_sizes_colors
 * @brief Validate the style values color or size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_sizes_colors(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;

    for (size_t i = 0; i < attribute->values->length; i++) {
        if (validate_style_value_color(styles, new_styles, attribute,
                                       allowed_values1,
                                       allowed_values2) == false &&
            validate_style_value_size(styles, new_styles, attribute,
                                      allowed_values1,
                                      allowed_values2) == false) {
            return false;
        }
    }

    return true;
}

/**
 *
 * @function validate_style_value_colors
 * @brief Validate the style values color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_colors(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;

    for (size_t i = 0; i < attribute->values->length; i++) {
        if (validate_style_value_color(styles, new_styles, attribute,
                                       allowed_values1,
                                       allowed_values2) == false) {
            return false;
        }
    }

    return true;
}

/**
 *
 * @function validate_style_value_color
 * @brief Validate the style value color
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_color(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }

    if (attribute->values == NULL || attribute->values->data == NULL ||
        attribute->values->data[0] == NULL) {
        return false;
    }

    ast_value_t *first = attribute->values->data[0];

    if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        } else if (value[0] == '#') {
            return true;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }
                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;
            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }
                i++;
            }
        }
    }

    return false;
}

/**
 *
 * @function validate_style_value_integer
 * @brief Validate the style value integer
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_integer(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }

    ast_value_t *first = attribute->values->data[0];

    if (first->type->kind == AST_TYPE_KIND_INT) {
        attribute->final_value = memory_allocate(20 * sizeof(char));
        snprintf(attribute->final_value, 20, "%d", first->data.int_value);

        return true;
    } else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
        if (first->data.float_value == (int)first->data.float_value) {
            attribute->final_value = memory_allocate(20 * sizeof(char));
            snprintf(attribute->final_value, 20, "%d",
                     (int)first->data.float_value);

            return true;
        }

        return false;
    } else if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }

                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;

            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }

                i++;
            }
        }

        if (string_is_integer(value)) {
            return true;
        }

        return false;
    }

    return false;
}

/**
 *
 * @function validate_style_value_number
 * @brief Validate the style value number
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_number(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }

    ast_value_t *first = attribute->values->data[0];

    if (first->type->kind == AST_TYPE_KIND_INT) {
        return true;
    } else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
        return true;
    } else if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }

                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;

            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }

                i++;
            }
        }

        if (string_is_number(value)) {
            return true;
        }

        return false;
    }

    return false;
}

/**
 *
 * @function validate_style_value_float
 * @brief Validate the style value float
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_float(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }

    ast_value_t *first = attribute->values->data[0];

    if (first->type->kind == AST_TYPE_KIND_INT) {
        return false;
    } else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
        return true;
    } else if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }

                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;

            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }

                i++;
            }
        }

        if (string_is_float(value)) {
            return true;
        }

        return false;
    }

    return false;
}

/**
 *
 * @function validate_style_value_percentage
 * @brief Validate the style value percentage
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_percentage(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }

    ast_value_t *first = attribute->values->data[0];

    if (first->type->kind == AST_TYPE_KIND_INT) {
        if (first->data.int_value <= 0 || first->data.int_value >= 100) {
            error_validator(2,
                            "Percentage value must be between 0 and 100 "
                            "for attribute '%s' "
                            "in '%s' element at line %zu column %zu!",
                            attribute->key,
                            ast_layout_node_type_to_enduser_name(
                                attribute->parent_node_type),
                            attribute->value_location.start_line,
                            attribute->value_location.start_column);

            return false;
        } else {
            attribute->final_value = memory_allocate(20 * sizeof(char));
            snprintf(attribute->final_value, 20, "%.2f",
                     first->data.int_value / 100.0);

            return true;
        }
    } else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
        if (first->data.float_value <= 0 || first->data.float_value >= 1) {
            error_validator(2,
                            "Percentage value must be between 0 and 100 "
                            "for attribute '%s' "
                            "in '%s' element at line %zu column %zu!",
                            attribute->key,
                            ast_layout_node_type_to_enduser_name(
                                attribute->parent_node_type),
                            attribute->value_location.start_line,
                            attribute->value_location.start_column);

            return false;
        } else {
            attribute->final_value = memory_allocate(20 * sizeof(char));
            snprintf(attribute->final_value, 20, "%.2f",
                     first->data.float_value);

            return true;
        }
    } else if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }

                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;

            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }

                i++;
            }
        }

        return true;  // TODO
    }

    return false;
}

/**
 *
 * @function validate_style_value_size
 * @brief Validate the style value size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_size(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2) {
    DEBUG_ME;
    if (styles) {
    }
    if (new_styles) {
    }
    if (allowed_values1) {
    }
    if (allowed_values2) {
    }

    ast_value_t *first = attribute->values->data[0];
    if (first->type->kind == AST_TYPE_KIND_INT) {
        attribute->final_value = memory_allocate(20 * sizeof(char));
        snprintf(attribute->final_value, 20, "%dpx", first->data.int_value);

        return true;
    } else if (first->type->kind == AST_TYPE_KIND_FLOAT) {
        attribute->final_value = memory_allocate(20 * sizeof(char));
        snprintf(attribute->final_value, 20, "%fpx", first->data.float_value);

        return true;
    } else if (first->type->kind == AST_TYPE_KIND_STRING) {
        char *value = first->data.string_value;

        if (strlen(value) == 0) {
            return false;
        }

        if (allowed_values2 != NULL) {
            size_t i = 0;

            while (allowed_values2[i].input != NULL) {
                if (strcmp(value, allowed_values2[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values2[i].output);

                    return true;
                }

                i++;
            }
        }

        if (allowed_values1 != NULL) {
            size_t i = 0;

            while (allowed_values1[i].input != NULL) {
                if (strcmp(value, allowed_values1[i].input) == 0) {
                    attribute->final_value =
                        string_strdup(allowed_values1[i].output);

                    return true;
                }

                i++;
            }
        }

        char *buffer = normalise_css_size(value);

        char *out_value;
        if (!has_css_size_prefix(buffer, &out_value)) {
            if (out_value != NULL) {
                memory_destroy(out_value);
            }

            memory_destroy(buffer);

            return false;
        }

        if (attribute->final_value != NULL) {
            memory_destroy(attribute->final_value);
        }

        attribute->final_value = string_strdup(out_value);

        memory_destroy(buffer);
        memory_destroy(out_value);

        return true;
    }

    return false;
}

/**
 *
 * @function validate_style_value_sizes
 * @brief Validate the style values size
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values1 -
 * Allowed values 1
 * @params {const ast_layout_attribute_style_pair_t*} allowed_values2 -
 * Allowed values 2
 * @params {bool} length_124 - Length 124
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value_sizes(
    hashmap_t *styles, hashmap_t *new_styles, ast_layout_attribute_t *attribute,
    const ast_layout_attribute_style_pair_t *allowed_values1,
    const ast_layout_attribute_style_pair_t *allowed_values2, bool length_124) {
    DEBUG_ME;

    if (length_124 == true) {
        if (attribute->values->length != 1 && attribute->values->length != 2 &&
            attribute->values->length != 4) {
            return false;
        }
    }

    for (size_t i = 0; i < attribute->values->length; i++) {
        if (validate_style_value_size(styles, new_styles, attribute,
                                      allowed_values1,
                                      allowed_values2) == false) {
            return false;
        }
    }

    return true;
}

/**
 *
 * @function validate_style_value
 * @brief Validate the style value
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New styles
 * @params {ast_layout_attribute_t*} attribute - Layout attribute
 * @returns {bool} - True if the style value is valid, false otherwise
 *
 */
bool validate_style_value(hashmap_t *styles, hashmap_t *new_styles,
                          ast_layout_attribute_t *attribute) {
    DEBUG_ME;
    ast_value_t *first = attribute->values->data[0];
    char *value = first->data.string_value;

    if (attribute->values->length < 1) {
        error_validator(
            2, "Style value is missing in '%s' element",
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
    // Global values
    else if (first->type->kind == AST_TYPE_KIND_STRING &&
             attribute->values->length == 1) {
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE(GENERATED_NAME, ENDUSER_NAME) \
    else if (strcmp(value, ENDUSER_NAME) == 0) {                              \
        return true;                                                          \
    }

#define ADD_LAYOUT_ATTRIBUTE_STYLE_GLOBAL_VALUE_REPEAT(GENERATED_NAME, \
                                                       ENDUSER_NAME)   \
    else if (strcmp(value, ENDUSER_NAME) == 0) {                       \
        return true;                                                   \
    }

        if (false) {
        }
#include "generated-config/ast_layout_attribute_style_global.h"
    }

    switch (attribute->type) {
#undef ADD_LAYOUT_ATTRIBUTE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, \
                                        GENERATED_NAME, FILTER,         \
                                        ALLOWED_VALUES, SUBTAGS)        \
    case TYPE:                                                            \
        return false;

#define ADD_LAYOUT_ATTRIBUTE_TYPE_REPEAT(TYPE, NAME, NAME_LOWER, ENDUSER_NAME, \
                                        GENERATED_NAME, FILTER,         \
                                        ALLOWED_VALUES, SUBTAGS)        \

#include "generated-config/ast_layout_attribute_type.h"

#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE
#undef ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE(TYPE, NAME, NAME_LOWER, ENDUSER_NAME,  \
                                        GENERATED_NAME, FILTER,                \
                                        ALLOWED_VALUES, SUBTAGS)               \
    case TYPE: {                                                               \
        attribute->final_key = string_strdup(GENERATED_NAME);                  \
        const ast_layout_attribute_style_pair_t *values = ALLOWED_VALUES;      \
                                                                               \
        if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_COLOR) {               \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_color(styles, new_styles, attribute,   \
                                              ast_layout_allowed_style_color,  \
                                              values);                         \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRING) {       \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_string(styles, new_styles, attribute,  \
                                               NULL, values);                  \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRING_ANY) {   \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_STRINGS_ANY) {  \
            attribute->final_value =                                           \
                array_value_stringify(attribute->values, ",");                 \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZE) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_size(styles, new_styles, attribute,    \
                                             NULL, values);                    \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZE_COLOR) {   \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_size_color(styles, new_styles,         \
                                                   attribute, NULL, values);   \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZES) {        \
            return validate_style_value_sizes(styles, new_styles, attribute,   \
                                              NULL, values, false);            \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZES124) {     \
            return validate_style_value_sizes(styles, new_styles, attribute,   \
                                              NULL, values, true);             \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_COLORS) {       \
            return validate_style_value_colors(styles, new_styles, attribute,  \
                                               NULL, values);                  \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_PERCENTAGE) {   \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_percentage(styles, new_styles,         \
                                                   attribute, NULL, values);   \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_SIZES_COLORS) { \
            return validate_style_value_sizes_colors(styles, new_styles,       \
                                                     attribute, NULL, values); \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_INTEGER) {      \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_integer(styles, new_styles, attribute, \
                                                NULL, values);                 \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_FLOAT) {        \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_float(styles, new_styles, attribute,   \
                                              NULL, values);                   \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_NUMBER) {       \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_number(styles, new_styles, attribute,  \
                                               NULL, values);                  \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_POSITIVE_INTEGER) {       \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_integer(styles, new_styles, attribute, \
                                                NULL, values);                 \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_POSITIVE_FLOAT) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_float(styles, new_styles, attribute,   \
                                              NULL, values);                   \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_POSITIVE_NUMBER) {        \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_number(styles, new_styles, attribute,  \
                                               NULL, values);                  \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_NEGATIVE_INTEGER) {       \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_integer(styles, new_styles, attribute, \
                                                NULL, values);                 \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_NEGATIVE_FLOAT) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_float(styles, new_styles, attribute,   \
                                              NULL, values);                   \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_NEGATIVE_NUMBER) {        \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return validate_style_value_number(styles, new_styles, attribute,  \
                                               NULL, values);                  \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_TIME) {         \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_PIXEL) {        \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER == AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_GROUP) {        \
            return true;                                                       \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_POSITIVE_PERCENTAGE) {    \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        } else if (FILTER ==                                                   \
                   AST_LAYOUY_ATTRIBUTE_STYLE_FILTER_NEGATIVE_PERCENTAGE) {    \
            if (attribute->values->length > 1) {                               \
                return false;                                                  \
            }                                                                  \
                                                                               \
            return true;                                                       \
        }                                                                      \
    }

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_REPEAT(                   \
    TYPE, NAME, NAME_LOWER, ENDUSER_NAME, GENERATED_NAME, FILTER, \
    ALLOWED_VALUES, SUBTAGS)

#define ADD_LAYOUT_ATTRIBUTE_STYLE_TYPE_HIDE(TYPE, NAME, NAME_LOWER,       \
                                             ENDUSER_NAME, GENERATED_NAME, \
                                             FILTER, ALLOWED_VALUES, SUBTAGS)

        default:
            return false;

#include "generated-config/ast_layout_attribute_style_type.h"
    }

    return false;
}
