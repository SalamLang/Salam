#include "token_keyword.h"

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
    {TOKEN_BOOL_TRUE, "true"},
    {TOKEN_BOOL_FALSE, "false"},

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
    {TOKEN_BOOL_TRUE, "درست"},
    {TOKEN_BOOL_FALSE, "غلط"},

    {TOKEN_EOF, NULL},
};
