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

#ifndef SALAM_LEXER_INTERNAL_H
#define SALAM_LEXER_INTERNAL_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "token/token.h"

typedef enum { LX_SUB_NAME, LX_SUB_VALUE } lx_layout_sub_t;

typedef struct {
    const char       *s;
    size_t            n;
    size_t            pos;
    uint32_t          line;
    uint32_t          col;
    const char       *file;
    int               group_depth;   /* '(' nesting only (general mode): newlines
                                       * are continuation inside parens, but are
                                       * SIGNIFICANT inside '[' ... ']' so a
                                       * multi-line array literal can separate its
                                       * elements by newline (comma-less). */
    token_kind_t      last;          /* last pushed kind (incl. STMT_END) */
    bool              has_last;
    bool              had_error;
    bool              keep_comments; /* emit TK_COMMENT_* instead of skipping
                                      * (set by the formatter; off for compiling) */
    bool              layout_mode;
    int               layout_depth;  /* open layout/element blocks */
    lx_layout_sub_t   layout_sub;    /* expecting a NAME or a VALUE */
    bool              comp_header;    /* inside a `component Name(params)` header,
                                       * awaiting the ':' that enters layout mode */
    arena_t          *a;
    logger_t         *log;
    const langpack_t *pack;
    token_stream_t   *out;
} lx_t;

#define LX_POS(L) ((src_pos_t){ (L)->line, (L)->col, (uint32_t)(L)->pos })

char        lx_peek(lx_t *L);

char        lx_peek2(lx_t *L, size_t k);

bool        lx_end(lx_t *L);

char        lx_adv(lx_t *L);

const char *lx_slice(lx_t *L, size_t start);

bool   lx_match(lx_t *L, char c);          /* consume one `c` if next; report whether */

void   lx_skip_ident(lx_t *L);             /* advance over an identifier-continue run  */

void   lx_skip_to_eol(lx_t *L);            /* advance to the next '\n' or EOF (excl.)  */

size_t lx_consume_hex(lx_t *L, int max);   /* advance over <= max hex digits; count    */

int    lx_at_digit(lx_t *L, size_t k);     /* byte len of decimal digit at +k, else 0  */

int    lx_take_digit(lx_t *L);             /* consume a decimal digit; value 0..9 / -1 */

bool   lx_expect(lx_t *L, char close, const src_pos_t *begin, const char *errmsg);

size_t lx_trim_ws_end(const char *s, size_t start, size_t end);

void lx_emit(lx_t *L, token_kind_t kind, const src_pos_t *begin, const char *lexeme);

void lx_emit_val(lx_t *L, token_kind_t kind, const src_pos_t *begin,

                 const char *lexeme, const token_value_t *val);

void lx_error(lx_t *L, const src_pos_t *at, const char *msg);

bool     lx_is_digit(char c);                       /* ASCII '0'..'9' (locale-free)    */

bool     lx_is_alpha(char c);                       /* ASCII letter (locale-free)      */

bool     lx_is_ident_start(char c);

bool     lx_is_ident_cont(char c);

int      lx_hex(char c);                            /* hex digit value, or -1          */

size_t   lx_utf8_encode(uint32_t cp, char *out);    /* codepoint -> UTF-8, byte count  */

int      lx_utf8_seq_len(const char *s, size_t n);  /* len of first UTF-8 seq, 0 = bad */

bool     lx_is_base_digit(char c, char base);       /* digit valid for x/b/o prefix    */

int      lx_decimal_digit(const char *s, size_t n, int *val); /* ASCII/Arabic/Persian 0-9 */

uint64_t lx_parse_uint(const char *d, int base, bool *overflow);

enum { LX_ESC_HEX_X = 2, LX_ESC_HEX_U = 4, LX_ESC_HEX_BIG_U = 8 };

void lx_newline_terminator(lx_t *L);  /* emit STMT_END for a significant newline       */

void lx_skip_trivia(lx_t *L);         /* skip whitespace + comments (general mode)      */

bool lx_try_line_comment(lx_t *L);    /* if at "//", consume it (+emit when keeping)    */

const char *lx_decode_string(lx_t *L, const char *lex, bool triple); /* "..."/"""...""" */

uint64_t    lx_decode_char(const char *lex);                          /* '...' -> value */

void lx_scan_number(lx_t *L);       /* INT / FLOAT, with decoded value              */

void lx_scan_string(lx_t *L);       /* STRING, decoded value (full escape support)  */

void lx_scan_triple(lx_t *L);       /* TRIPLE_STRING ("""…""" raw)                 */

void lx_scan_raw_string(lx_t *L);   /* RAW_STRING (`…` backtick, no escapes)       */

void lx_scan_char(lx_t *L);         /* CHAR, decoded byte/codepoint value           */

void lx_scan_utf8_char(lx_t *L);    /* UTF8_CHAR (u'…' single codepoint)           */

void lx_scan_ident(lx_t *L);        /* IDENT or keyword (+ bool value)              */

void lx_scan_meta(lx_t *L);         /* @meta localization annotation                */

void lx_scan_op(lx_t *L);           /* operators / delimiters (+ raw-block capture) */

void lx_layout_step(lx_t *L);       /* one layout-mode action                       */

#endif /* SALAM_LEXER_INTERNAL_H */
