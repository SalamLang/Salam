/*
 * Salam Programming Language (2024-2026)
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

#include "core/sal_path.h"

bool sal_path_is_sep(char c)
{
    return c == '/' || c == '\\';
}

bool sal_path_is_absolute(const char *p)
{
    if (!p || !p[0]) return false;
    if (sal_path_is_sep(p[0])) return true;
    if (((p[0] >= 'A' && p[0] <= 'Z') || (p[0] >= 'a' && p[0] <= 'z')) && p[1] == ':')
        return true;
    return false;
}

char *sal_path_to_slash(char *p)
{
    char *q = p;
    if (!p) return p;
    for (; *q; q++)
        if (*q == '\\') *q = '/';
    return p;
}

/* "a//b" -> "a/b". Starts at index 2 so a leading "//" survives: on Windows
   that is a UNC root ("//server/share"), where the doubling is the syntax. */
static void collapse_double_sep(char *p)
{
    size_t i;
    if (strlen(p) < 3) return;
    for (i = 2; p[i];) {
        if (p[i] == '/' && p[i - 1] == '/')
            memmove(p + i, p + i + 1, strlen(p + i + 1) + 1);
        else
            i++;
    }
}

char *sal_path_normalize(char *p)
{
    char *q, *dd;
    if (!p) return p;
    sal_path_to_slash(p);
    collapse_double_sep(p);

    while (p[0] == '.' && p[1] == '/')
        memmove(p, p + 2, strlen(p + 2) + 1);

    while ((q = strstr(p, "/./")) != NULL)
        memmove(q + 1, q + 3, strlen(q + 3) + 1);

    dd = p;
    while ((dd = strstr(dd, "/../")) != NULL) {
        char *seg = dd;
        size_t seglen;
        while (seg > p && seg[-1] != '/')
            seg--;
        seglen = (size_t)(dd - seg);
        /* never collapse a ".." segment, an empty segment ("//../", a
           rooted "/../") or a drive prefix like "c:" - skip past those */
        if (seglen == 0 || (seglen == 2 && seg[0] == '.' && seg[1] == '.') ||
            memchr(seg, ':', seglen) != NULL) {
            dd += 1;
            continue;
        }
        memmove(seg, dd + 4, strlen(dd + 4) + 1);
        dd = p;
    }
    return p;
}

char *sal_path_trim_sep(char *p)
{
    size_t n;
    if (!p) return p;
    n = strlen(p);
    /* "/" and "c:/" are roots: the separator is the whole path, not a
       trailing one, and dropping it would turn them into "" and "c:". */
    while (n > 1 && sal_path_is_sep(p[n - 1]) && !(n == 3 && p[1] == ':'))
        p[--n] = '\0';
    return p;
}

const char *sal_path_base(const char *p)
{
    const char *cut = NULL;
    const char *q = p;
    if (!p) return p;
    for (; *q; q++)
        if (sal_path_is_sep(*q)) cut = q;
    return cut ? cut + 1 : p;
}

/* Copy at most n-1 bytes of [s, s+len) into out and terminate. Returns len,
   so the caller sees what it would have needed. */
static size_t copy_bounded(char *out, size_t n, const char *s, size_t len)
{
    size_t take = len;
    if (n == 0) return len;
    if (take >= n) take = n - 1;
    memcpy(out, s, take);
    out[take] = '\0';
    return len;
}

static size_t stem_len(const char *base)
{
    const char *dot = strrchr(base, '.');
    return dot ? (size_t)(dot - base) : strlen(base);
}

size_t sal_path_stem_buf(const char *p, char *out, size_t n)
{
    const char *base = sal_path_base(p);
    return copy_bounded(out, n, base, stem_len(base));
}

const char *sal_path_stem(arena_t *a, const char *p)
{
    const char *base = sal_path_base(p);
    return arena_strndup(a, base, stem_len(base));
}

static size_t dir_len(const char *p)
{
    const char *cut = NULL;
    const char *q = p;
    for (; *q; q++)
        if (sal_path_is_sep(*q)) cut = q;
    return cut ? (size_t)(cut - p) : 0;
}

size_t sal_path_dir_buf(const char *p, char *out, size_t n)
{
    size_t len = dir_len(p);
    if (n == 0) return len;
    copy_bounded(out, n, p, len);
    sal_path_to_slash(out);
    return len;
}

const char *sal_path_dir(arena_t *a, const char *p)
{
    size_t len = dir_len(p);
    if (len == 0) return "";
    return sal_path_to_slash((char *)arena_strndup(a, p, len));
}

/* Length `dir` keeps once its trailing separators are dropped - what
   sal_path_trim_sep would leave, computed without a buffer so join can
   report the size it needed even when it had to truncate. */
static size_t trimmed_len(const char *dir)
{
    size_t n = strlen(dir);
    while (n > 1 && sal_path_is_sep(dir[n - 1]) && !(n == 3 && dir[1] == ':'))
        n--;
    return n;
}

size_t sal_path_join(char *out, size_t n, const char *dir, const char *name)
{
    size_t need, have;
    if (!name) name = "";
    /* A leading separator on `name` would double up against the one added
       here; the caller means "under dir", not "at the root". */
    while (sal_path_is_sep(name[0]))
        name++;
    if (n == 0) return (dir && dir[0] ? trimmed_len(dir) + 1 : 0) + strlen(name);

    if (!dir || !dir[0]) {
        copy_bounded(out, n, name, strlen(name));
        sal_path_to_slash(out);
        return strlen(name);
    }

    copy_bounded(out, n, dir, trimmed_len(dir));
    sal_path_to_slash(out);
    need = trimmed_len(dir);
    if (!name[0]) return need;
    need += 1 + strlen(name);
    have = strlen(out);
    if (have + 1 < n) {
        out[have] = '/';
        copy_bounded(out + have + 1, n - have - 1, name, strlen(name));
        sal_path_to_slash(out);
    }
    return need;
}

const char *sal_path_joina(arena_t *a, const char *dir, const char *name)
{
    size_t cap = (dir ? strlen(dir) : 0) + (name ? strlen(name) : 0) + 2;
    char *out = (char *)arena_alloc(a, cap);
    sal_path_join(out, cap, dir, name);
    return out;
}
