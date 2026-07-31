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

#include "minify/minify.h"
#include "core/sb.h"

static bool js_word_char(int c)
{
    return isalnum(c) || c == '_' || c == '$';
}

static size_t js_copy_string(sb_t *out, const char *src, size_t i, size_t n)
{
    char q = src[i];
    sb_putc(out, q);
    i++;
    while (i < n) {
        char c = src[i];
        if (c == '\\' && i + 1 < n) {
            sb_putc(out, c);
            sb_putc(out, src[i + 1]);
            i += 2;
            continue;
        }
        sb_putc(out, c);
        i++;
        if (c == q) break;
    }
    return i;
}

static size_t js_copy_template(sb_t *out, const char *src, size_t i, size_t n);

static size_t js_copy_expr_hole(sb_t *out, const char *src, size_t i, size_t n)
{
    int depth = 1;
    sb_putc(out, src[i]);
    sb_putc(out, src[i + 1]);
    i += 2;
    while (i < n && depth > 0) {
        char c = src[i];
        if (c == '{') {
            depth++;
            sb_putc(out, c);
            i++;
            continue;
        }
        if (c == '}') {
            depth--;
            sb_putc(out, c);
            i++;
            continue;
        }
        if (c == '"' || c == '\'') {
            i = js_copy_string(out, src, i, n);
            continue;
        }
        if (c == '`') {
            i = js_copy_template(out, src, i, n);
            continue;
        }
        sb_putc(out, c);
        i++;
    }
    return i;
}

static size_t js_copy_template(sb_t *out, const char *src, size_t i, size_t n)
{
    sb_putc(out, src[i]);
    i++;
    while (i < n) {
        char c = src[i];
        if (c == '\\' && i + 1 < n) {
            sb_putc(out, c);
            sb_putc(out, src[i + 1]);
            i += 2;
            continue;
        }
        if (c == '`') {
            sb_putc(out, c);
            i++;
            break;
        }
        if (c == '$' && i + 1 < n && src[i + 1] == '{') {
            i = js_copy_expr_hole(out, src, i, n);
            continue;
        }
        sb_putc(out, c);
        i++;
    }
    return i;
}

static size_t js_copy_regex(sb_t *out, const char *src, size_t i, size_t n)
{
    size_t start = i;
    bool in_class = false;
    i++;
    while (i < n) {
        char c = src[i];
        if (c == '\\' && i + 1 < n) {
            i += 2;
            continue;
        }
        if (c == '[') {
            in_class = true;
            i++;
            continue;
        }
        if (c == ']') {
            in_class = false;
            i++;
            continue;
        }
        if (c == '/' && !in_class) {
            i++;
            break;
        }
        if (c == '\n') break;
        i++;
    }
    while (i < n && isalpha((unsigned char)src[i]))
        i++;
    {
        size_t k = start;
        for (; k < i; k++)
            sb_putc(out, src[k]);
    }
    return i;
}

static bool js_ends_with_word(const char *s, size_t len, const char *word)
{
    size_t wl = strlen(word);
    if (len < wl) return false;
    if (strncmp(s + len - wl, word, wl) != 0) return false;
    if (len > wl) {
        char before = s[len - wl - 1];
        if (js_word_char((unsigned char)before)) return false;
    }
    return true;
}

static bool js_regex_allowed(sb_t *out)
{
    size_t j = out->len;
    while (j > 0 && isspace((unsigned char)out->data[j - 1]))
        j--;
    if (j == 0) return true;
    {
        char c = out->data[j - 1];
        if (js_word_char((unsigned char)c)) {
            static const char *const kw[] = {
                "return", "typeof", "instanceof", "in",    "of", "new",
                "delete", "void",   "throw",      "case",  "do", "else",
                "yield",  "await",  "if",         "while", NULL};
            int k = 0;
            for (; kw[k]; k++)
                if (js_ends_with_word(out->data, j, kw[k])) return true;
            return false;
        }
        if (c == ')' || c == ']') return false;
        return true;
    }
}

static bool js_needs_space(char prevc, char nextc)
{
    if (!prevc || !nextc) return false;
    if (js_word_char((unsigned char)prevc) && js_word_char((unsigned char)nextc))
        return true;
    if (isdigit((unsigned char)prevc) && nextc == '.') return true;
    if (prevc == '.' && isdigit((unsigned char)nextc)) return true;
    {
        static const char *const pairs[] = {"++", "--", "+=", "-=", "*=", "/=", "%=",
                                            "==", "!=", "<=", ">=", "&&", "||", "**",
                                            "=>", "<<", ">>", "??", "?.", "..", NULL};
        int k = 0;
        for (; pairs[k]; k++)
            if (pairs[k][0] == prevc && pairs[k][1] == nextc) return true;
    }
    if (prevc == '/' && (nextc == '/' || nextc == '*')) return true;
    if (prevc == '*' && nextc == '/') return true;
    return false;
}

const char *minify_js(arena_t *arena, const char *src)
{
    sb_t out;
    size_t n = strlen(src);
    size_t i = 0;
    sb_init(&out);
    while (i < n) {
        char c = src[i];
        if (c == '/' && i + 1 < n && src[i + 1] == '/') {
            i += 2;
            while (i < n && src[i] != '\n')
                i++;
            continue;
        }
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            const char *end = strstr(src + i + 2, "*/");
            i = end ? (size_t)(end - src) + 2 : n;
            continue;
        }
        if (c == '"' || c == '\'') {
            i = js_copy_string(&out, src, i, n);
            continue;
        }
        if (c == '`') {
            i = js_copy_template(&out, src, i, n);
            continue;
        }
        if (c == '/' && js_regex_allowed(&out)) {
            i = js_copy_regex(&out, src, i, n);
            continue;
        }
        if (isspace((unsigned char)c)) {
            size_t j = i;
            char prevc, nextc;
            while (j < n && isspace((unsigned char)src[j]))
                j++;
            prevc = out.len ? out.data[out.len - 1] : 0;
            nextc = j < n ? src[j] : 0;
            if (js_needs_space(prevc, nextc)) sb_putc(&out, ' ');
            i = j;
            continue;
        }
        sb_putc(&out, c);
        i++;
    }
    while (out.len && isspace((unsigned char)out.data[out.len - 1]))
        out.len--;
    {
        const char *res = arena_strdup(arena, sb_cstr(&out));
        sb_free(&out);
        return res;
    }
}

const char *minify_css(arena_t *arena, const char *src)
{
    sb_t out;
    size_t n = strlen(src);
    size_t i = 0;
    int paren_depth = 0;
    sb_init(&out);
    while (i < n) {
        char c = src[i];
        if (c == '/' && i + 1 < n && src[i + 1] == '*') {
            const char *end = strstr(src + i + 2, "*/");
            i = end ? (size_t)(end - src) + 2 : n;
            continue;
        }
        if (c == '\'' || c == '"') {
            char q = c;
            sb_putc(&out, c);
            i++;
            while (i < n) {
                char d = src[i];
                if (d == '\\' && i + 1 < n) {
                    sb_putc(&out, d);
                    sb_putc(&out, src[i + 1]);
                    i += 2;
                    continue;
                }
                sb_putc(&out, d);
                i++;
                if (d == q) break;
            }
            continue;
        }
        if (c == '(') paren_depth++;
        if (c == ')' && paren_depth > 0) paren_depth--;
        if (isspace((unsigned char)c)) {
            size_t j = i;
            char prevc, nextc;
            bool drop;
            while (j < n && isspace((unsigned char)src[j]))
                j++;
            prevc = out.len ? out.data[out.len - 1] : 0;
            nextc = j < n ? src[j] : 0;
            if (paren_depth > 0) {
                drop = !prevc || prevc == '(' || nextc == ')';
            } else {
                drop = !prevc || !nextc || strchr("{}:;,>+~(", prevc) != NULL ||
                       strchr("{}:;,>+~)", nextc) != NULL;
            }
            if (!drop) sb_putc(&out, ' ');
            i = j;
            continue;
        }
        if (c == '}') {
            if (out.len && out.data[out.len - 1] == ';') out.len--;
            if (out.len && out.data[out.len - 1] == ' ') out.len--;
        } else if ((c == '{' || c == ':' || c == ';' || c == ',' || c == '>' ||
                    c == '+' || c == '~') &&
                   paren_depth == 0) {
            if (out.len && out.data[out.len - 1] == ' ') out.len--;
        }
        sb_putc(&out, c);
        i++;
    }
    while (out.len && isspace((unsigned char)out.data[out.len - 1]))
        out.len--;
    {
        const char *res = arena_strdup(arena, sb_cstr(&out));
        sb_free(&out);
        return res;
    }
}
