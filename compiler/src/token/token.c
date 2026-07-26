/*
 * Salam Programming Language (2024–2026)
 *
 *   +-------------------+
 *   |     S A L A M     |
 *   +-------------------+
 *
 * Designed by Seyyed Ali Mohammadiyeh and the Salam Team
 * Born from a decade of language design experience (since 2018)
 *
 * Repository: https://github.com/SalamLang/Salam
 *
 */

#include "core/prelude.h"
#include "token/token.h"
#include "core/vec.h"

static const char *const k_token_names[TK__COUNT] = {
    [TK_EOF] = "EOF",
    [TK_STMT_END] = "STMT_END",
    [TK_IDENT] = "IDENTIFIER",
    [TK_INT] = "INT_LITERAL",
    [TK_FLOAT] = "FLOAT_LITERAL",
    [TK_STRING] = "STRING_LITERAL",
    [TK_TRIPLE_STRING] = "TRIPLE_STRING_LITERAL",
    [TK_RAW_STRING] = "RAW_STRING_LITERAL",
    [TK_CHAR] = "CHAR_LITERAL",
    [TK_UTF8_CHAR] = "UTF8_CHAR_LITERAL",
    [TK_KW_FUNC] = "KEYWORD_FUNC",
    [TK_KW_RET] = "KEYWORD_RET",
    [TK_KW_IF] = "KEYWORD_IF",
    [TK_KW_ELSE] = "KEYWORD_ELSE",
    [TK_KW_UNTIL] = "KEYWORD_UNTIL",
    [TK_KW_ON] = "KEYWORD_ON",
    [TK_KW_MUT] = "KEYWORD_MUT",
    [TK_KW_CONST] = "KEYWORD_CONST",
    [TK_KW_TYPE] = "KEYWORD_TYPE",
    [TK_KW_STRUCT] = "KEYWORD_STRUCT",
    [TK_KW_ENUM] = "KEYWORD_ENUM",
    [TK_KW_END] = "KEYWORD_END",
    [TK_KW_IMPORT] = "KEYWORD_IMPORT",
    [TK_KW_AS] = "KEYWORD_AS",
    [TK_KW_TRUE] = "KEYWORD_TRUE",
    [TK_KW_FALSE] = "KEYWORD_FALSE",
    [TK_KW_NULL] = "KEYWORD_NULL",
    [TK_KW_THIS] = "KEYWORD_THIS",
    [TK_KW_BREAK] = "KEYWORD_BREAK",
    [TK_KW_CONTINUE] = "KEYWORD_CONTINUE",
    [TK_KW_LAYOUT] = "KEYWORD_LAYOUT",
    [TK_KW_PACKAGE] = "KEYWORD_PACKAGE",
    [TK_KW_PRINT] = "KEYWORD_PRINT",
    [TK_KW_PRINTLN] = "KEYWORD_PRINTLN",
    [TK_KW_PRINTERR] = "KEYWORD_PRINTERR",
    [TK_KW_PRINTERRLN] = "KEYWORD_PRINTERRLN",
    [TK_KW_INPUT] = "KEYWORD_INPUT",
    [TK_KW_DEFER] = "KEYWORD_DEFER",
    [TK_KW_OPERATOR] = "KEYWORD_OPERATOR",
    [TK_KW_EXTERN] = "KEYWORD_EXTERN",
    [TK_KW_INTERFACE] = "KEYWORD_INTERFACE",
    [TK_KW_PUB] = "KEYWORD_PUB",
    [TK_KW_INLINE] = "KEYWORD_INLINE",
    [TK_KW_NOINLINE] = "KEYWORD_NOINLINE",
    [TK_KW_PURE] = "KEYWORD_PURE",
    [TK_KW_NORET] = "KEYWORD_NORET",
    [TK_KW_DEPRECATED] = "KEYWORD_DEPRECATED",
    [TK_KW_COMPONENT] = "KEYWORD_COMPONENT",
    [TK_KW_REPEAT] = "KEYWORD_REPEAT",
    [TK_KW_IMPL] = "KEYWORD_IMPL",
    [TK_KW_TO] = "KEYWORD_TO",
    [TK_KW_STEP] = "KEYWORD_STEP",
    [TK_KW_EACH] = "KEYWORD_EACH",
    [TK_KW_IN] = "KEYWORD_IN",
    [TK_KW_WITH] = "KEYWORD_WITH",
    [TK_KW_MATCH] = "KEYWORD_MATCH",
    [TK_PLUS] = "PLUS",
    [TK_MINUS] = "MINUS",
    [TK_STAR] = "STAR",
    [TK_SLASH] = "SLASH",
    [TK_PERCENT] = "PERCENT",
    [TK_POWER] = "POWER",
    [TK_EQ] = "EQ",
    [TK_NE] = "NE",
    [TK_LT] = "LT",
    [TK_GT] = "GT",
    [TK_LE] = "LE",
    [TK_GE] = "GE",
    [TK_AND] = "AND",
    [TK_OR] = "OR",
    [TK_NOT] = "NOT",
    [TK_AMP] = "AMP",
    [TK_PIPE] = "PIPE",
    [TK_CARET] = "CARET",
    [TK_TILDE] = "TILDE",
    [TK_SHL] = "SHL",
    [TK_SHR] = "SHR",
    [TK_ASSIGN] = "ASSIGN",
    [TK_COLON_ASSIGN] = "COLON_ASSIGN",
    [TK_PLUS_EQ] = "PLUS_ASSIGN",
    [TK_MINUS_EQ] = "MINUS_ASSIGN",
    [TK_STAR_EQ] = "STAR_ASSIGN",
    [TK_SLASH_EQ] = "SLASH_ASSIGN",
    [TK_PERCENT_EQ] = "PERCENT_ASSIGN",
    [TK_POWER_EQ] = "POWER_ASSIGN",
    [TK_AMP_EQ] = "AMP_ASSIGN",
    [TK_PIPE_EQ] = "PIPE_ASSIGN",
    [TK_CARET_EQ] = "CARET_ASSIGN",
    [TK_SHL_EQ] = "SHL_ASSIGN",
    [TK_SHR_EQ] = "SHR_ASSIGN",
    [TK_PLUS_PLUS] = "PLUS_PLUS",
    [TK_MINUS_MINUS] = "MINUS_MINUS",
    [TK_LPAREN] = "LPAREN",
    [TK_RPAREN] = "RPAREN",
    [TK_LBRACE] = "LBRACE",
    [TK_RBRACE] = "RBRACE",
    [TK_LBRACKET] = "LBRACKET",
    [TK_RBRACKET] = "RBRACKET",
    [TK_COLON] = "COLON",
    [TK_QUESTION] = "QUESTION",
    [TK_COMMA] = "COMMA",
    [TK_DOT] = "DOT",
    [TK_SEMICOLON] = "SEMICOLON",
    [TK_LAYOUT_ELEMENT] = "LAYOUT_ELEMENT",
    [TK_LAYOUT_ATTR_NAME] = "LAYOUT_ATTR_NAME",
    [TK_META] = "META",
    [TK_FAT_ARROW] = "FAT_ARROW",
    [TK_ELLIPSIS] = "ELLIPSIS",
    [TK_COMMENT_LINE] = "COMMENT_LINE",
    [TK_COMMENT_BLOCK] = "COMMENT_BLOCK",
};

const char *token_kind_name(token_kind_t kind)
{
    if (kind < 0 || kind >= TK__COUNT || !k_token_names[kind]) return "UNKNOWN";
    return k_token_names[kind];
}

struct token_stream {
    vec_t v;
    const char *file;
};

token_stream_t *token_stream_new(arena_t *a, const char *file)
{
    token_stream_t *ts = (token_stream_t *)arena_alloc(a, sizeof(*ts));
    vec_init(&ts->v);
    ts->file = file;
    return ts;
}

void token_stream_push(arena_t *a, token_stream_t *ts, const token_t *tok)
{
    token_t *slot = (token_t *)arena_alloc(a, sizeof(*slot));
    *slot = *tok;
    vec_push(a, &ts->v, slot);
}

size_t token_stream_count(const token_stream_t *ts)
{
    return ts->v.len;
}

const token_t *token_stream_at(const token_stream_t *ts, size_t i)
{
    return (const token_t *)ts->v.data[i];
}

const char *token_stream_file(const token_stream_t *ts)
{
    return ts->file;
}
