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

#ifndef _GENERATOR_LAYOUT_H_
#define _GENERATOR_LAYOUT_H_

#include <stddef.h>

#include "ast.h"
#include "ast_layout.h"
#include "ast_layout_style.h"
#include "base.h"
#include "generator.h"
#include "generator_layout_style.h"
#include "memory.h"
#include "string_buffer.h"

/**
 *
 * @function attribute_value_handler
 * @brief Handles the value of a layout attribute and transforms it based on its
 * type.
 * @param attribute Pointer to the layout attribute structure.
 * @param value The string value of the attribute to process.
 * @return A dynamically allocated string containing the processed value, or
 * NULL if the handler type is unsupported.
 *
 */
char *attribute_value_handler(ast_layout_attribute_t *attribute, char *value);

/**
 *
 * @function generator_code_layout_value
 * @brief Convert AST layout attribute values to CSS attribute values
 * @params {hashmap_t*} styles - Styles
 * @params {hashmap_t*} new_styles - New Styles
 * @params {ast_layout_attribute_t*} attribute - Layout Attribute
 * @returns {void}
 *
 */
void generator_code_layout_value(hashmap_t *styles, hashmap_t *new_styles,
                                 ast_layout_attribute_t *attribute);

/**
 *
 * @function generator_code_layout_block
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @params {array_t*} children - Children
 * @returns {string_t*}
 *
 */
string_t *generator_code_layout_block(generator_t *generator,
                                      array_t *children);
/**
 *
 * @function generator_code_layout_body
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} body - Body
 * @returns {void}
 *
 */
void generator_code_layout_body(generator_t *generator,
                                ast_layout_block_t *layout_block,
                                string_t *body);

/**
 *
 * @function generator_code_head_item
 * @params {ast_layout_attribute_t*} attribute - Attribute
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head_item(ast_layout_attribute_t *attribute,
                              string_t *head);
/**
 *
 * @function generator_code_head
 * @brief Generate the code for the head
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head(generator_t *generator, ast_layout_block_t *block,
                         string_t *head);
/**
 *
 * @function generator_code_layout
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator - Generator
 * @returns {void}
 *
 */
void generator_code_layout(generator_t *generator);

/**
 *
 * @function generator_code_layout_html
 * @brief Generate the HTML code for the layout block
 * @params {generator_t*} generator
 * @params {ast_layout_block_t*} layout_block - Layout block
 * @params {string_t*} html - HTML string
 * @returns {void}
 *
 */
void generator_code_layout_html(generator_t *generator,
                                ast_layout_block_t *layout_block,
                                string_t *html);
/**
 *
 * @function generator_code_layout_attributes
 * @brief Generate the HTML code for the layout block attributes
 * @params {generator_t} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @params {bool} notIgnoreBelongsToLayout
 * @returns {string_t*}
 *
 */
string_t *generator_code_layout_attributes(generator_t *generator,
                                           ast_layout_block_t *block,
                                           bool notIgnoreBelongsToLayout);

/**
 *
 * @function generator_code_head_meta_children
 * @brief Generate the CSS/HTML code for meta children in the layout
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_block_t*} block - Layout block
 * @params {string_t*} head - Head
 * @returns {void}
 *
 */
void generator_code_head_meta_children(generator_t *generator,
                                       ast_layout_block_t *block,
                                       string_t *head);

/**
 *
 * @function generator_code_layout_block_item
 * @brief Generate the HTML code for the layout block item
 * @params {generator_t*} generator - Generator
 * @params {ast_layout_node_t*} node - Node
 * @returns {string_t*}
 *
 */
string_t *generator_code_layout_block_item(generator_t *generator,
                                           ast_layout_node_t *node);

// TODO
bool is_generator_code_head(ast_layout_attribute_t *attribute);

#endif
