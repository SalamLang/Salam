#ifndef SALAM_LANGPACK_INTERNAL_H
#define SALAM_LANGPACK_INTERNAL_H
#include "core/prelude.h"
#include "langpack/langpack.h"
#include "token/token.h"

typedef struct { const char *canon; const char *spelling; } ctx_entry_t;
struct langpack {
    const char       *code;
    const kw_entry_t *keywords;
    const char       *entry;       /* entry-point function name (-> C `main`) */
    const ctx_entry_t *contextual; /* localized contextual words, or NULL */
};
extern struct langpack g_lang_en;
extern struct langpack g_lang_fa;
extern const kw_entry_t k_lang_en[];
#endif /* SALAM_LANGPACK_INTERNAL_H */
