#ifndef _LEXER_TOKEN_KEYWORD_H_
#define _LEXER_TOKEN_KEYWORD_H_

#include "token.h"

typedef struct {
    token_type_t type;
    const char *localized;
} token_keyword_t;

#endif
