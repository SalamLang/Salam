#ifndef _VALIDATOR_H_
#define _VALIDATOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "base.h"
#include "ast.h"
#include "hashmap.h"

/**
 * 
 * @function validate_layout_block
 * @brief Validate and modify the layout block
 * @params {ast_layout_block_t*} block - Layout block
 * @returns {void}
 * 
 */
void validate_layout_block(ast_layout_block_t* block);

#endif
