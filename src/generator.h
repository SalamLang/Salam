#ifndef aGENERATOR_H_
#define _GENERATOR_H_

#include <stddef.h>

#include "memory.h"
#include "string.h"

typedef struct {
	string_t* html;
	string_t* css;
	string_t* js;
} generator_t;

/**
 * 
 * @function generator_create
 * @brief Create a generator
 * @params {}
 * @returns {generator_t*}
 * 
 */
generator_t* generator_create();

/**
 * 
 * @function generator_destroy
 * @brief Destroy a generator
 * @params {generator_t*} generator
 * @returns {void}
 * 
 */
void generator_destroy(generator_t* generator);

/**
 * 
 * @function generator_debug
 * @brief Debug the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 * 
 */
void generator_debug(generator_t* generator);

/**
 * 
 * @function generator_save
 * @brief Save the generator
 * @params {generator_t*} generator - Generator
 * @returns {void}
 * 
 */
void generator_save(generator_t* generator);

#endif
