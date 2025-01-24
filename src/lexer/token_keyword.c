#include "token_keyword.h"

token_keyword_t keywords[] = {
    // Comparison Operators
    {TOKEN_EQUAL, "=="},
    {TOKEN_NOT_EQUAL, "!="},
    {TOKEN_LESS, "<"},
    {TOKEN_GREATER, ">"},
    {TOKEN_LESS_EQUAL, "<="},
    {TOKEN_GREATER_EQUAL, ">="},
    
    // Logical Operators
    {TOKEN_AND_AND, "&&"},
    {TOKEN_OR_OR, "||"},
    {TOKEN_NOT, "!"},
    
    // Bitwise Operators
    {TOKEN_AND_BIT, "&"},
    {TOKEN_OR_BIT, "|"},
    {TOKEN_XOR_BIT, "^"},
    {TOKEN_SHIFT_LEFT, "<<"},
    {TOKEN_SHIFT_RIGHT, ">>"},
    
    // Assignment Operators
    {TOKEN_ASSIGN, "="},
    {TOKEN_SHIFT_LEFT_ASSIGN, "<<="},
    {TOKEN_SHIFT_RIGHT_ASSIGN, ">>="},
    
    // Parentheses and Braces
    {TOKEN_LEFT_PAREN, "("},
    {TOKEN_RIGHT_PAREN, ")"},
    {TOKEN_LEFT_BRACE, "{"},
    {TOKEN_RIGHT_BRACE, "}"},
    {TOKEN_LEFT_BRACKET, "["},
    {TOKEN_RIGHT_BRACKET, "]"},
    
    // Other Symbols
    {TOKEN_COMMA, ","},
    {TOKEN_COLON, ":"},

    {TOKEN_BLOCK_BEGIN, ":"},
    {TOKEN_EOF, NULL},
};

token_keyword_t en_keywords[] = {
    {TOKEN_IF, "if"},
    {TOKEN_ELSE, "else"},
    {TOKEN_PRINT, "print"},
    {TOKEN_FN, "fn"},
    {TOKEN_REPEAT, "repeat"},
    {TOKEN_RET, "ret"},
    {TOKEN_BREAK, "break"},
    {TOKEN_CONOTINUE, "continue"},
    {TOKEN_LAYOUT, "layout"},
    {TOKEN_BLOCK_END, "end"},
    {TOKEN_EOF, NULL},
};

token_keyword_t fa_keywords[] = {
    {TOKEN_IF, "اگر"},
    {TOKEN_ELSE, "وگرنه"},
    {TOKEN_PRINT, "چاپ"},
    {TOKEN_FN, "تابع"},
    {TOKEN_REPEAT, "تکرار"},
    {TOKEN_RET, "برگشت"},
    {TOKEN_BREAK, "شکستن"},
    {TOKEN_CONOTINUE, "ادامه"},
    {TOKEN_LAYOUT, "صفحه"},
    {TOKEN_BLOCK_END, "پایان"},
    {TOKEN_EOF, NULL},
};
