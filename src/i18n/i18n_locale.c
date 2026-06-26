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
