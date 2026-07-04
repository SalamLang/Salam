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
 * Per-file language detection. Every source file is lexed and type-checked in
 * its own language: the stdlib (English keywords) stays English while a Persian
 * program stays Persian. The language is taken from an explicit marker comment
 * when present, otherwise inferred by counting how many keywords each candidate
 * language recognises.
 */

#include "core/prelude.h"
#include "lexer/lexer.h"
#include "langpack/langpack.h"
#include "token/token.h"

/* Match the ASCII tag at [p,le) case-insensitively; return its length or 0. */
static size_t det_match_ci(const char *text, size_t p, size_t le,
                           const char *tag, size_t n)
{
    size_t k = 0;
    if (p + n > le) return 0;
    for (; k < n; k++) {
        char ch = text[p + k];
        if (ch >= 'A' && ch <= 'Z') ch = (char)(ch - 'A' + 'a');
        if (ch != tag[k]) return 0;
    }
    return n;
}

/* Match a raw byte sequence at [p,le); return its length or 0. */
static size_t det_match_bytes(const char *text, size_t p, size_t le,
                              const char *tag, size_t n)
{
    size_t k = 0;
    if (p + n > le) return 0;
    for (; k < n; k++) if (text[p + k] != tag[k]) return 0;
    return n;
}

/*
 * Read the language marker in the first few comment lines and return its code
 * ("fa"/"en") or NULL. A Persian file marks itself fully in Persian
 * (`// زبان: فارسی`), an English file fully in English (`// lang: en` /
 * `// language: english`). Mismatched spellings such as `// LANG: fa` or
 * `// زبان: fa` are intentionally rejected so each file stays in one language.
 */
const char *langpack_marker_code(const char *text, size_t len)
{
    static const char en_tag1[] = "lang";          /* 4 bytes */
    static const char en_tag2[] = "language";      /* 8 bytes */
    static const char en_word[] = "english";       /* 7 bytes */
    /* Persian words as raw UTF-8 so this source stays ASCII-only. */
    static const char fa_tag[] = "\xd8\xb2\xd8\xa8\xd8\xa7\xd9\x86";              /* زبان    (8) */
    static const char fa_val[] = "\xd9\x81\xd8\xa7\xd8\xb1\xd8\xb3\xdb\x8c";      /* فارسی   (10) */
    static const char en_val[] = "\xd8\xa7\xd9\x86\xda\xaf\xd9\x84\xdb\x8c"
                                 "\xd8\xb3\xdb\x8c";                             /* انگلیسی (14) */
    size_t i = 0; int line = 0;
    while (i < len && line < 8) {
        size_t ls = i;
        while (ls < len && (text[ls] == ' ' || text[ls] == '\t')) ls++;
        size_t le = ls;
        while (le < len && text[le] != '\n') le++;
        if (ls + 1 < len && text[ls] == '/' && text[ls + 1] == '/') {
            size_t p = ls + 2;
            for (; p < le; p++) {
                int fa_tagged = 0;
                size_t tlen = det_match_bytes(text, p, le, fa_tag, 8);
                if (tlen) fa_tagged = 1;
                if (!tlen) tlen = det_match_ci(text, p, le, en_tag2, 8);
                if (!tlen) tlen = det_match_ci(text, p, le, en_tag1, 4);
                if (!tlen) continue;
                size_t q = p + tlen;
                while (q < le && (text[q] == ' ' || text[q] == '\t')) q++;
                if (q >= le || text[q] != ':') continue;
                q++;
                while (q < le && (text[q] == ' ' || text[q] == '\t')) q++;
                if (fa_tagged) {
                    /* Persian tag accepts only Persian value words. */
                    if (det_match_bytes(text, q, le, fa_val, 10)) return "fa";
                    if (det_match_bytes(text, q, le, en_val, 14)) return "en";
                    return NULL;
                }
                /* English tag accepts only English values. */
                if (det_match_ci(text, q, le, en_word, 7)) return "en";
                {
                    size_t cs = q;
                    while (q < le && ((text[q] >= 'a' && text[q] <= 'z') ||
                                      (text[q] >= 'A' && text[q] <= 'Z'))) q++;
                    if (q - cs == 2 && (text[cs] == 'e' || text[cs] == 'E') &&
                        (text[cs + 1] == 'n' || text[cs + 1] == 'N')) return "en";
                }
                return NULL;
            }
        }
        i = (le < len) ? le + 1 : le;
        line++;
    }
    return NULL;
}

static int det_count_keywords(arena_t *a, logger_t *quiet,
                              const langpack_t *pack, const source_file_t *src)
{
    token_stream_t *toks = NULL;
    lexer_run(a, quiet, pack, src, &toks);
    if (!toks) return 0;
    int count = 0;
    size_t n = token_stream_count(toks);
    { size_t i = 0; for (; i < n; i++) {
        const token_t *t = token_stream_at(toks, i);
        if (t->kind == TK_EOF) break;
        if (tk_is_keyword(t->kind)) count++;
    } }
    return count;
}

const langpack_t *langpack_detect(arena_t *a, const source_file_t *src,
                                  const langpack_t *fallback)
{
    const char *marked = langpack_marker_code(src->text, src->len);
    if (marked) {
        const langpack_t *p = langpack_load(marked);
        if (p) return p;
    }
    static const char *codes[] = { "en", "fa" };
    logger_t *quiet = logger_new(stderr, LOG_OFF, false);
    const langpack_t *best = fallback; int best_kw = 0;
    { size_t i = 0; for (; i < sizeof codes / sizeof codes[0]; i++) {
        const langpack_t *p = langpack_load(codes[i]);
        if (!p) continue;
        int kw = det_count_keywords(a, quiet, p, src);
        if (kw > best_kw) { best_kw = kw; best = p; }
    } }
    logger_free(quiet);
    return best;
}
