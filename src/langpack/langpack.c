#include "core/prelude.h"
#include "langpack/langpack.h"
#include "langpack/langpack_internal.h"

langpack_t *langpack_load(const char *code)
{
    if (code && strcmp(code, "en") == 0) return &g_lang_en;
    if (code && strcmp(code, "fa") == 0) return &g_lang_fa;
    return NULL;
}

void langpack_free(langpack_t *p)
{
    (void)p; 
}

const char *langpack_code(const langpack_t *p)
{
    return p->code;
}

const char *langpack_entry(const langpack_t *p)
{
    return p->entry;
}

const char *langpack_entry_for(const char *code)
{
    langpack_t *p = langpack_load(code);
    return p ? p->entry : "main";
}

const char *langpack_end_spelling(const langpack_t *p)
{
    for (const kw_entry_t *e = p->keywords; e->spelling != NULL; e++)
        if (e->kind == TK_KW_END) return e->spelling;
    return "end";
}
