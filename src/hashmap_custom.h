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

#ifndef _HASHMAP_CUSTOM_H_
#define _HASHMAP_CUSTOM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "array.h"
#include "hashmap.h"
#include "memory.h"

/**
 *
 * @function hashmap_print_layout_attribute
 * @brief Print the hashmap of layout attributes
 * @params {ast_layout_attribute_t*} map - The hashmap to print
 * @returns {void}
 *
 */
void hashmap_print_layout_attribute(hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_destroy_layout_attribute
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy_layout_attribute(hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_create_layout_attribute
 * @brief Create a new hashmap of layout attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_t*}
 *
 */
hashmap_layout_attribute_t *hashmap_create_layout_attribute(size_t capacity);

/**
 *
 * @function hashmap_destroy_layout_attribute_style_state
 * @brief Destroy the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_destroy_layout_attribute_style_state(
    hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_print_layout_attribute_style_state
 * @brief Print the hashmap of layout style state attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_print_layout_attribute_style_state(
    hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_create_layout_attribute_style_state
 * @brief Create a new hashmap of layout style state attributes
 * @params {size_t} capacity
 * @returns {hashmap_layout_attribute_state_style_t*}
 *
 */
hashmap_layout_attribute_state_style_t *
hashmap_create_layout_attribute_style_state(size_t capacity);

/**
 *
 * @function hashmap_layout_attribute_print
 * @brief Print the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_layout_attribute_print(hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_layout_attribute_destroy
 * @brief Destroy the hashmap of layout attributes
 * @params {hashmap_layout_attribute_t*} map
 * @returns {void}
 *
 */
void hashmap_layout_attribute_destroy(hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_has_any_sub_value_layout_attribute_style_state
 * @brief Check if the hashmap has any sub value layout attribute style state
 * @params {hashmap_layout_attribute_t*} map
 * @returns {bool}
 *
 */
bool hashmap_has_any_sub_value_layout_attribute_style_state(
    hashmap_layout_attribute_t *map);

/**
 *
 * @function hashmap_has_any_sub_value_layout_attribute
 * @brief Check if the hashmap has any sub value layout attribute
 * @params {hashmap_layout_attribute_t*} map
 * @returns {bool}
 *
 */
bool hashmap_layout_attribute_has_any_sub_value(
    hashmap_layout_attribute_t *map);

#endif
