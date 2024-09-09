#ifndef _GENERATOR_SALAM_H_
#define _GENERATOR_SALAM_H_

#include "ast.h"
#include "generator.h"
#include "string_buffer.h"

/**
 *
 * @function generator_salam
 * @brief Generate the Salam code (cleaned and formatted)
 * @params {ast_t*} ast - AST
 * @returns {string_t*}
 *
 */
string_t* generator_salam(ast_t* ast);

#endif
