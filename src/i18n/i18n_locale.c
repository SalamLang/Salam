#include "core/prelude.h"
#include "i18n/i18n.h"
#include "i18n/i18n_internal.h"

static const char *g_lang = "en";
void i18n_set_lang(const char *code)
{
    if (code && strcmp(code, "fa") == 0) g_lang = "fa";
    else                                 g_lang = "en";
}

const char *i18n_lang(void) { return g_lang; }

const char *i18n_active_lang(void) { return g_lang; }
