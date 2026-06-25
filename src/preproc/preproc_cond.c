#include "core/prelude.h"
#include "preproc/preproc_internal.h"

const char *pp_skip_ws(const char *p) { while (*p == ' ' || *p == '\t') p++; return p; }

static bool pp_defined(const char *const *defs, int n, const char *name, size_t len)
{
    for (int i = 0; i < n; i++)
        if (strlen(defs[i]) == len && strncmp(defs[i], name, len) == 0) return true;
    return false;
}

static const char *pp_get_value(const char *const *defs, int n,
                                const char *name, size_t namelen)
{
    for (int i = 0; i < n; i++)
        if (strncmp(defs[i], name, namelen) == 0 && defs[i][namelen] == '=')
            return defs[i] + namelen + 1;
    return NULL;
}

static bool is_ident_char(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') || c == '_';
}

bool pp_eval(const char *cond, const char *const *defs, int n)
{
    cond = pp_skip_ws(cond);
    bool neg = false;
    if (*cond == '!') { neg = true; cond = pp_skip_ws(cond + 1); }
    if (strncmp(cond, "defined(", 8) == 0) {
        const char *name = cond + 8;
        const char *rb = strchr(name, ')');
        size_t len = rb ? (size_t)(rb - name) : strlen(name);
        while (len > 0 && (name[len-1] == ' ' || name[len-1] == '\t')) len--;
        bool v = pp_defined(defs, n, name, len);
        return neg ? !v : v;
    }
    
    const char *name = cond;
    const char *e = name;
    while (is_ident_char(*e)) e++;
    size_t len = (size_t)(e - name);
    
    const char *after = pp_skip_ws(e);
    if (len > 0 && (strncmp(after, "==", 2) == 0 || strncmp(after, "!=", 2) == 0)) {
        bool is_eq = (after[0] == '=');
        after = pp_skip_ws(after + 2);
        if (*after == '"') {
            after++;
            const char *ve = strchr(after, '"');
            size_t vlen = ve ? (size_t)(ve - after) : strlen(after);
            const char *def_val = pp_get_value(defs, n, name, len);
            bool match = def_val != NULL && strlen(def_val) == vlen &&
                         strncmp(def_val, after, vlen) == 0;
            bool v = is_eq ? match : !match;
            return neg ? !v : v;
        }
    }
    bool v = pp_defined(defs, n, name, len);
    return neg ? !v : v;
}
