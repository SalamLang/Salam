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

#ifndef SALAM_TOKEN_TOKEN_H
#define SALAM_TOKEN_TOKEN_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/span.h"
#include "xml/xml.h"

typedef enum token_kind_t {
    TK_EOF = 0,
    TK_STMT_END,
    TK_IDENT,
    TK_INT,
    TK_FLOAT,
    TK_STRING,
    TK_TRIPLE_STRING,
    TK_RAW_STRING,
    TK_CHAR,
    TK_UTF8_CHAR,
    TK_KW_FUNC,
    TK_KW_RET,
    TK_KW_IF,
    TK_KW_ELSE,
    TK_KW_UNTIL,
    TK_KW_ON,
    TK_KW_MUT,
    TK_KW_CONST,
    TK_KW_TYPE,
    TK_KW_STRUCT,
    TK_KW_ENUM,
    TK_KW_END,
    TK_KW_IMPORT,
    TK_KW_AS,
    TK_KW_TRUE,
    TK_KW_FALSE,
    TK_KW_NULL,
    TK_KW_THIS,
    TK_KW_BREAK,
    TK_KW_CONTINUE,
    TK_KW_LAYOUT,
    TK_KW_PACKAGE,
    TK_KW_PRINT,
    TK_KW_PRINTLN,
    TK_KW_PRINTERR,
    TK_KW_PRINTERRLN,
    TK_KW_INPUT,
    TK_KW_DEFER,
    TK_KW_OPERATOR,
    TK_KW_EXTERN,
    TK_KW_INTERFACE,
    TK_KW_PUB,
    TK_KW_INLINE,
    TK_KW_NOINLINE,
    TK_KW_PURE,
    TK_KW_NORET,
    TK_KW_DEPRECATED,
    TK_KW_COMPONENT,
    TK_KW_REPEAT,
    TK_KW_IMPL,
    TK_KW_TO,
    TK_KW_STEP,
    TK_KW_EACH,
    TK_KW_IN,
    TK_KW_WITH,
    TK_KW_MATCH,
    TK_PLUS,
    TK_MINUS,
    TK_STAR,
    TK_SLASH,
    TK_PERCENT,
    TK_POWER,
    TK_EQ,
    TK_NE,
    TK_LT,
    TK_GT,
    TK_LE,
    TK_GE,
    TK_AND,
    TK_OR,
    TK_NOT,
    TK_AMP,
    TK_PIPE,
    TK_CARET,
    TK_TILDE,
    TK_SHL,
    TK_SHR,
    TK_ASSIGN,
    TK_COLON_ASSIGN,
    TK_PLUS_EQ,
    TK_MINUS_EQ,
    TK_STAR_EQ,
    TK_SLASH_EQ,
    TK_PERCENT_EQ,
    TK_POWER_EQ,
    TK_AMP_EQ,
    TK_PIPE_EQ,
    TK_CARET_EQ,
    TK_SHL_EQ,
    TK_SHR_EQ,
    TK_PLUS_PLUS,
    TK_MINUS_MINUS,
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACE,
    TK_RBRACE,
    TK_LBRACKET,
    TK_RBRACKET,
    TK_COLON,
    TK_QUESTION,
    TK_COMMA,
    TK_DOT,
    TK_SEMICOLON,
    TK_FAT_ARROW,
    TK_ELLIPSIS,
    TK_LAYOUT_ELEMENT,
    TK_LAYOUT_ATTR_NAME,
    TK_META,
    TK_COMMENT_LINE,
    TK_COMMENT_BLOCK,
    TK__COUNT
} token_kind_t;

SAL_INLINE bool tk_is_keyword(token_kind_t k)
{
    return k >= TK_KW_FUNC && k <= TK_KW_MATCH;
}

typedef enum {
    TV_NONE = 0,
    TV_INT,
    TV_FLOAT,
    TV_STRING,
    TV_CHAR,
    TV_BOOL
} token_value_kind_t;

typedef struct {
    token_value_kind_t kind;
    union {
        uint64_t i;
        double f;
        const char *s;
        bool b;
    } as;
    size_t slen;
} token_value_t;

typedef struct {
    token_kind_t kind;
    const char *lexeme;
    src_span_t span;
    token_value_t value;
    bool layout_mode;
} token_t;

const char *token_kind_name(token_kind_t kind);

typedef struct token_stream token_stream_t;

token_stream_t *token_stream_new(arena_t *a, const char *file);

void token_stream_push(arena_t *a, token_stream_t *ts, const token_t *tok);

size_t token_stream_count(const token_stream_t *ts);

const token_t *token_stream_at(const token_stream_t *ts, size_t i);

const char *token_stream_file(const token_stream_t *ts);

void tokens_to_xml(xml_writer_t *w, const token_stream_t *ts);

#endif /* SALAM_TOKEN_TOKEN_H */
