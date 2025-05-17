#ifndef _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_PARAMETERS_PARSER_PARSE_PARAMETERS_H_
#define _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_PARAMETERS_PARSER_PARSE_PARAMETERS_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// array
#include <utility/array/type.h>
#include <utility/array/array_create/array_create.h>

// parser
#include <stage/parser/type.h>

// token
#include <stage/token/type.h>

// ast
#include <stage/ast/type.h>
#include <stage/ast/asts/ast_parameter/ast_parameters/ast_parameters_create/ast_parameters_create.h>

ast_t* parser_parse_parameters(parser_t* parser);

#endif // _STAGE_PARSER_PARSER_PARSE_PARSER_PARSE_PARAMETERS_PARSER_PARSE_PARAMETERS_H_
