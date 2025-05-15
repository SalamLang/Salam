#ifndef _STAGE_PARSER_PARSER_CREATE_PARSER_CREATE_H_
#define _STAGE_PARSER_PARSER_CREATE_PARSER_CREATE_H_

// base
#include <base.h>

// memory
#include <utility/memory/memory_allocation/memory_allocation.h>

// parser
#include <stage/parser/type.h>

// scanner
#include <stage/scanner/type.h>

parser_t* parser_create(const scanner_t* scanner);

#endif // _STAGE_PARSER_PARSER_CREATE_PARSER_CREATE_H_
