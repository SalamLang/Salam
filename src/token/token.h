#ifndef SALAM_TOKEN_TOKEN_H
#define SALAM_TOKEN_TOKEN_H
#include "core/prelude.h"
#include "core/arena.h"
#include "core/span.h"
#include "xml/xml.h"

typedef enum token_kind_t {
    TK_EOF = 0,
    TK_STMT_END,        /* ';' or a significant newline (also a layout line break) */
    TK_IDENT,
    TK_INT,
    TK_FLOAT,
    TK_STRING,
    TK_TRIPLE_STRING,
    TK_RAW_STRING,      /* `...`  backtick raw string (no escape processing)    */
    TK_CHAR,
    TK_UTF8_CHAR,       /* u'...' single UTF-8 codepoint (stored as TV_STRING)  */
    TK_KW_FUNC, TK_KW_RET, TK_KW_IF, TK_KW_ELSE, TK_KW_WHILE, TK_KW_FOR,
    TK_KW_MUT, TK_KW_CONST, TK_KW_TYPE, TK_KW_STRUCT, TK_KW_ENUM,
    TK_KW_END, TK_KW_IMPORT, TK_KW_AS, TK_KW_TRUE, TK_KW_FALSE, TK_KW_NULL,
    TK_KW_THIS, TK_KW_BREAK, TK_KW_CONTINUE, TK_KW_LAYOUT, TK_KW_PACKAGE,
    TK_KW_PRINT, TK_KW_PRINTLN, TK_KW_PRINTERR, TK_KW_PRINTERRLN, /* R1: print family */
    TK_KW_INPUT,               /* R1 (resolved): keyword tokens */
    TK_KW_DEFER,
    TK_KW_OPERATOR,   /* contextual keyword for operator overloading */
    TK_KW_EXTERN,     /* `extern "C"` FFI declaration block */
    TK_KW_INTERFACE,  /* `interface` behavioral contract (generic bound) */
    TK_KW_PUB,        /* `pub` / `عمومی`: marks a package symbol as exported */
    TK_KW_COMPONENT,  /* `component` / `مولفه`: a named, parameterized layout fragment */
    TK_KW_REPEAT,     /* `repeat` / `تکرار`: counted loop (`repeat N: ... end`) */
    TK_KW_IMPL,       /* `impl` / `پیاده‌سازی`: `impl Iface for Type:` method block */
    TK_KW_TO,         /* `to` / `تا`: range upper bound in `repeat A to B:` */
    TK_KW_STEP,       /* `every` / `هر`: range stride in `repeat A to B every S:` */
    TK_PLUS, TK_MINUS, TK_STAR, TK_SLASH, TK_PERCENT, TK_POWER,
    TK_EQ, TK_NE, TK_LT, TK_GT, TK_LE, TK_GE, TK_AND, TK_OR, TK_NOT,
    TK_AMP,               /* & (single ampersand: pass-by-reference param marker)   */
    TK_ASSIGN, TK_PLUS_EQ, TK_MINUS_EQ, TK_STAR_EQ, TK_SLASH_EQ, TK_PERCENT_EQ,
    TK_LPAREN, TK_RPAREN, TK_LBRACE, TK_RBRACE, TK_LBRACKET, TK_RBRACKET,
    TK_COLON, TK_COMMA, TK_DOT, TK_SEMICOLON,
    TK_FAT_ARROW,         /* => (lambda) */
    TK_ELLIPSIS,          /* ... (C variadic marker in extern declarations) */
    TK_LAYOUT_ELEMENT,    /* element name up to ':'  */
    TK_LAYOUT_ATTR_NAME,  /* attribute name up to '=' */
    TK_META,              /* `@lang` localization annotation; lexeme = lang code */
    TK_COMMENT_LINE,      /* `// ...` to end of line (trailing whitespace trimmed) */
    TK_COMMENT_BLOCK,     /* `/`+`* ... *`+`/` block comment (may span lines)       */
    TK__COUNT
} token_kind_t;
static inline bool tk_is_keyword(token_kind_t k) {
    return k >= TK_KW_FUNC && k <= TK_KW_STEP;
}
typedef enum {
    TV_NONE = 0, TV_INT, TV_FLOAT, TV_STRING, TV_CHAR, TV_BOOL
} token_value_kind_t;
typedef struct {
    token_value_kind_t kind;
    union {
        uint64_t    i;   /* TV_INT, TV_CHAR (byte/codepoint) */
        double      f;   /* TV_FLOAT */
        const char *s;   /* TV_STRING (decoded, arena) */
        bool        b;   /* TV_BOOL  */
    } as;
} token_value_t;
typedef struct {
    token_kind_t  kind;
    const char   *lexeme;      /* raw arena slice of the source (NUL-terminated) */
    src_span_t    span;
    token_value_t value;       /* decoded literal value (TV_NONE if not a literal) */
    bool          layout_mode; /* true if scanned inside a layout: block */
} token_t;
const char *token_kind_name(token_kind_t kind);
typedef struct token_stream token_stream_t;
token_stream_t *token_stream_new(arena_t *a, const char *file);
void            token_stream_push(arena_t *a, token_stream_t *ts, const token_t *tok);
size_t          token_stream_count(const token_stream_t *ts);
const token_t  *token_stream_at(const token_stream_t *ts, size_t i);
const char     *token_stream_file(const token_stream_t *ts);
void tokens_to_xml(xml_writer_t *w, const token_stream_t *ts);
#endif /* SALAM_TOKEN_TOKEN_H */
