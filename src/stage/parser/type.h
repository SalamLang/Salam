#ifndef _STAGE_PARSER_TYPE_H_
#define _STAGE_PARSER_TYPE_H_

#include <stddef.h> // for size_t

// base
#include <base.h>

// scanner
#include <stage/scanner/type.h>

typedef struct parser_t
{
    const scanner_t* scanner; // Pointer to the scanner
    size_t index;             // Current index in the tokens array
} parser_t;

#define PARSER_CURRENT       ((token_t*)parser->scanner->tokens->items[parser->index])
#define PARSER_CURRENT_NEXT  ((token_t*)parser->scanner->tokens->items[parser->index + 1])
#define PARSER_CURRENT_PREV  ((token_t*)parser->scanner->tokens->items[parser->index - 1])

#define PARSER_HAS_CURRENT   (parser->index < parser->scanner->tokens->size && PARSER_CURRENT != NULL && PARSER_CURRENT->type != TOKEN_TYPE_EOF)
#define PARSER_HAS_NEXT      (parser->index + 1 < parser->scanner->tokens->size && PARSER_CURRENT_NEXT != NULL)
#define PARSER_HAS_PREV      (parser->index > 0)


#define PARSER_NEXT parser->index++
#define PARSER_PREV parser->index--

#endif // _STAGE_PARSER_TYPE_H_
