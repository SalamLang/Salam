#include "core/prelude.h"
#include "langpack/langpack.h"
#include "langpack/langpack_internal.h"

token_kind_t langpack_lookup_keyword(const langpack_t *p, const char *ident)
{
    for (const kw_entry_t *e = p->keywords; e->spelling != NULL; e++) {
        if (strcmp(e->spelling, ident) == 0) return e->kind;
    }
    
    if (p->keywords != k_lang_en)
        for (const kw_entry_t *e = k_lang_en; e->spelling != NULL; e++)
            if (strcmp(e->spelling, ident) == 0) return e->kind;
    return TK_IDENT;
}

const char *langpack_canon_word(const langpack_t *p, const char *ident)
{
    if (!p->contextual) return NULL;     
    for (const ctx_entry_t *e = p->contextual; e->spelling != NULL; e++)
        if (strcmp(e->spelling, ident) == 0) return e->canon;
    return NULL;
}
