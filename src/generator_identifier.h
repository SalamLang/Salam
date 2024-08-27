#ifndef _GENERATOR_IDENTIFIER_H_
#define _GENERATOR_IDENTIFIER_H_

#include "base.h"
#include "memory.h"
#include "string.h"

typedef struct generator_identifier_t
{
	char *current;
} generator_identifier_t;

/**
 *
 * @function generator_identifier_init
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 *
 */
void generator_identifier_init(generator_identifier_t *gen);

/**
 *
 * @function generator_identifier_get
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {char*} identifier - Identifier
 *
 */
char *generator_identifier_get(generator_identifier_t *gen);

/**
 *
 * @function generator_identifier_destroy
 * @params {generator_identifier_t*} gen - Generator Identifier
 * @returns {void}
 *
 */
void generator_identifier_destroy(generator_identifier_t *gen);

#endif
