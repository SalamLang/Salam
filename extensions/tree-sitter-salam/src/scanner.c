/*
 * External scanner for tree-sitter-salam.
 *
 * Salam reads words that stand next to each other on one line as a single
 * name ("fn ty", "عدد اول") and stops at a keyword or at the end of the line,
 * so names cannot be a regular-expression token. The scanner reads them word
 * by word against the compiler's keyword list in keywords.h. Where the
 * compiler reads keywords as plain words, the grammar asks for a different
 * token:
 *
 *   NAME         an ordinary name; stops at a keyword
 *   MEMBER_NAME  a name after '.', which may start with a keyword
 *                ("رشته.پایان با")
 *   LAYOUT_NAME  an element tag or attribute name in a layout block, read up
 *                to ':' or '=' whatever the words are ("word break",
 *                "border block end color")
 *   IMPORT_PATH  the package after 'import', up to the end of the line or an
 *                'as'
 *
 * 'page', 'component' and 'dyn' are soft keywords: they are names wherever
 * the parser cannot take the keyword, which is what valid_symbols says.
 */

#include "tree_sitter/parser.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "keywords.h"

enum TokenType {
    NAME,
    MEMBER_NAME,
    LAYOUT_NAME,
    IMPORT_PATH,
    LAYOUT_KEYWORD,
    COMPONENT_KEYWORD,
    DYN_KEYWORD
};

#define MAX_WORD 256

static bool is_digit(int32_t c)
{
    return (c >= '0' && c <= '9') || (c >= 0x0660 && c <= 0x0669) ||
           (c >= 0x06F0 && c <= 0x06F9);
}

static bool is_punct(int32_t c)
{
    return c == 0x060C || c == 0x061B || c == 0x061F || c == 0x066B || c == 0x066C ||
           c == 0x00AB || c == 0x00BB || c == 0x00A0 || c == 0xFEFF;
}

static bool is_start(int32_t c)
{
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        return true;
    }
    return c >= 0x80 && !is_digit(c) && !is_punct(c) && c != 0x200C && c != 0x200D &&
           !(c >= 0x2000 && c <= 0x206F);
}

static bool is_continue(int32_t c)
{
    return is_start(c) || is_digit(c) || c == 0x200C || c == 0x200D ||
           (c >= 0x064B && c <= 0x065F) || c == 0x0670;
}

static bool is_space(int32_t c)
{
    return c == ' ' || c == '\t';
}

static int put_utf8(char *buf, int len, int32_t c)
{
    if (len + 4 >= MAX_WORD) {
        return len;
    }
    if (c < 0x80) {
        buf[len++] = (char)c;
    } else if (c < 0x800) {
        buf[len++] = (char)(0xC0 | (c >> 6));
        buf[len++] = (char)(0x80 | (c & 0x3F));
    } else if (c < 0x10000) {
        buf[len++] = (char)(0xE0 | (c >> 12));
        buf[len++] = (char)(0x80 | ((c >> 6) & 0x3F));
        buf[len++] = (char)(0x80 | (c & 0x3F));
    } else {
        buf[len++] = (char)(0xF0 | (c >> 18));
        buf[len++] = (char)(0x80 | ((c >> 12) & 0x3F));
        buf[len++] = (char)(0x80 | ((c >> 6) & 0x3F));
        buf[len++] = (char)(0x80 | (c & 0x3F));
    }
    return len;
}

static bool in_table(const char *const *table, size_t count, const char *word, int len)
{
    for (size_t i = 0; i < count; i++) {
        if ((int)strlen(table[i]) == len && memcmp(table[i], word, (size_t)len) == 0) {
            return true;
        }
    }
    return false;
}

static bool is_keyword(const char *w, int n)
{
    return in_table(SALAM_KEYWORDS, SALAM_KEYWORD_COUNT, w, n);
}

static int soft_kind(const char *w, int n)
{
    if (in_table(SALAM_LAYOUT_WORDS, SALAM_LAYOUT_WORDS_COUNT, w, n)) {
        return LAYOUT_KEYWORD;
    }
    if (in_table(SALAM_COMPONENT_WORDS, SALAM_COMPONENT_WORDS_COUNT, w, n)) {
        return COMPONENT_KEYWORD;
    }
    if (n == 3 && memcmp(w, "dyn", 3) == 0) {
        return DYN_KEYWORD;
    }
    return -1;
}

static bool is_as(const char *w, int n)
{
    return in_table(SALAM_AS_WORDS, SALAM_AS_WORDS_COUNT, w, n);
}

static bool is_end(const char *w, int n)
{
    return in_table(SALAM_END_WORDS, SALAM_END_WORDS_COUNT, w, n);
}

static int read_word(TSLexer *lexer, char *buf)
{
    int len = 0;
    while (is_continue(lexer->lookahead)) {
        len = put_utf8(buf, len, lexer->lookahead);
        lexer->advance(lexer, false);
    }
    return len;
}

static void skip_blank(TSLexer *lexer)
{
    while (is_space(lexer->lookahead) || lexer->lookahead == '\n' ||
           lexer->lookahead == '\r' || lexer->lookahead == 0xFEFF) {
        lexer->advance(lexer, true);
    }
}

static void join_words(TSLexer *lexer, char *word, bool member)
{
    bool pending = false;
    for (;;) {
        bool spaced = false;
        while (is_space(lexer->lookahead)) {
            lexer->advance(lexer, false);
            spaced = true;
        }
        if (pending && lexer->lookahead == '(') {
            lexer->mark_end(lexer);
            return;
        }
        if (!spaced || !is_start(lexer->lookahead)) {
            return;
        }
        int len = read_word(lexer, word);
        if (len == 0) {
            return;
        }
        if (is_keyword(word, len)) {
            if (lexer->lookahead == '.' || (member && lexer->lookahead == '(')) {
                lexer->mark_end(lexer);
                return;
            }
            if (!member) {
                return;
            }
            pending = true;
            continue;
        }
        if (!pending) {
            lexer->mark_end(lexer);
        } else if (lexer->lookahead == '(') {
            lexer->mark_end(lexer);
            return;
        }
    }
}

static bool scan_import_path(TSLexer *lexer)
{
    char word[MAX_WORD];
    bool any = false;
    for (;;) {
        while (is_space(lexer->lookahead)) {
            lexer->advance(lexer, false);
        }
        int32_t c = lexer->lookahead;
        if (c == '\n' || c == '\r' || c == 0 || c == '/' || c == '"') {
            break;
        }
        if (is_start(c)) {
            int len = read_word(lexer, word);
            if (any && is_as(word, len)) {
                break;
            }
            any = true;
            lexer->mark_end(lexer);
        } else if (c == '.' || is_digit(c)) {
            lexer->advance(lexer, false);
            lexer->mark_end(lexer);
        } else {
            break;
        }
    }
    if (!any) {
        return false;
    }
    lexer->result_symbol = IMPORT_PATH;
    return true;
}

static bool scan_layout_name(TSLexer *lexer)
{
    char word[MAX_WORD];
    int words = 0;
    bool first_is_end = false;
    for (;;) {
        int len = read_word(lexer, word);
        if (len == 0) {
            return false;
        }
        if (words == 0) {
            first_is_end = is_end(word, len);
        }
        words++;
        lexer->mark_end(lexer);
        while (is_space(lexer->lookahead)) {
            lexer->advance(lexer, false);
        }
        int32_t c = lexer->lookahead;
        if (c == ':' || c == '=') {
            lexer->result_symbol = LAYOUT_NAME;
            return true;
        }
        if (!is_start(c) || (first_is_end && words == 1)) {
            return false;
        }
    }
}

static bool scan_member_name(TSLexer *lexer)
{
    char word[MAX_WORD];
    int len = read_word(lexer, word);
    if (len == 0) {
        return false;
    }
    lexer->mark_end(lexer);
    join_words(lexer, word, true);
    lexer->result_symbol = MEMBER_NAME;
    return true;
}

static bool scan_name(TSLexer *lexer, const bool *valid_symbols)
{
    char word[MAX_WORD];
    int len = read_word(lexer, word);
    if (len == 0) {
        return false;
    }
    if (len == 1 && word[0] == 'u' && lexer->lookahead == '\'') {
        return false;
    }
    int soft = soft_kind(word, len);
    if (soft >= 0) {
        lexer->mark_end(lexer);
        bool keyword = valid_symbols[soft];
        if (keyword && valid_symbols[NAME]) {
            while (is_space(lexer->lookahead)) {
                lexer->advance(lexer, false);
            }
            int32_t c = lexer->lookahead;
            if (c == ':') {
                lexer->advance(lexer, false);
                keyword = lexer->lookahead != '=';
            } else {
                keyword = is_start(c) || c == '(';
            }
        }
        if (keyword) {
            lexer->result_symbol = (TSSymbol)soft;
            return true;
        }
        if (!valid_symbols[NAME]) {
            return false;
        }
        lexer->result_symbol = NAME;
        return true;
    }
    if (!valid_symbols[NAME]) {
        return false;
    }
    if (is_keyword(word, len)) {
        return false;
    }
    lexer->mark_end(lexer);
    join_words(lexer, word, false);
    lexer->result_symbol = NAME;
    return true;
}

void *tree_sitter_salam_external_scanner_create(void)
{
    return NULL;
}

void tree_sitter_salam_external_scanner_destroy(void *payload)
{
    (void)payload;
}

unsigned tree_sitter_salam_external_scanner_serialize(void *payload, char *buffer)
{
    (void)payload;
    (void)buffer;
    return 0;
}

void tree_sitter_salam_external_scanner_deserialize(void *payload, const char *buffer,
                                                    unsigned length)
{
    (void)payload;
    (void)buffer;
    (void)length;
}

bool tree_sitter_salam_external_scanner_scan(void *payload, TSLexer *lexer,
                                             const bool *valid_symbols)
{
    (void)payload;
    bool recovering = valid_symbols[NAME] && valid_symbols[MEMBER_NAME] &&
                      valid_symbols[LAYOUT_NAME] && valid_symbols[IMPORT_PATH] &&
                      valid_symbols[DYN_KEYWORD];
    skip_blank(lexer);
    if (!recovering && valid_symbols[IMPORT_PATH]) {
        return scan_import_path(lexer);
    }
    if (!is_start(lexer->lookahead)) {
        return false;
    }
    if (!recovering && valid_symbols[MEMBER_NAME]) {
        return scan_member_name(lexer);
    }
    if (!recovering && valid_symbols[LAYOUT_NAME]) {
        return scan_layout_name(lexer);
    }
    if (valid_symbols[NAME] || valid_symbols[LAYOUT_KEYWORD] ||
        valid_symbols[COMPONENT_KEYWORD] || valid_symbols[DYN_KEYWORD]) {
        return scan_name(lexer, valid_symbols);
    }
    return false;
}
