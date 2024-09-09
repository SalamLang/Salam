#include "generator_salam.h"

/**
 *
 * @function generator_salam
 * @brief Generate the Salam code (cleaned and formatted)
 * @params {ast_t*} ast - AST
 * @returns {string_t*}
 *
 */
string_t* generator_salam(ast_t* ast) {
    DEBUG_ME;
    string_t* salam = string_create(1024);

    generator_salam_code(ast, salam);

    return salam;
}
