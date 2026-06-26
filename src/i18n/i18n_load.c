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

#define I18N_DYN_MAX 512
#define I18N_DYN_VAL_MAX 256

static int g_dyn_words_n = 0;

static int g_dyn_vals_n = 0;

static const i18n_pair_t k_value_fa[] = {
    { NULL, NULL }
};

static const i18n_pair_t k_layout_fa[] = {
    { NULL, NULL }
};

static struct { const char *lang, *alias, *canon; } g_dyn_vals[I18N_DYN_VAL_MAX];

static struct { const char *lang, *alias, *canon; } g_dyn_words[I18N_DYN_MAX];

void i18n_register_layout_word(const char *lang, const char *alias, const char *canonical)
{
    if (!lang || !alias || !canonical || g_dyn_words_n >= I18N_DYN_MAX) return;
    g_dyn_words[g_dyn_words_n].lang  = lang;
    g_dyn_words[g_dyn_words_n].alias = alias;
    g_dyn_words[g_dyn_words_n].canon = canonical;
    g_dyn_words_n++;
}

const char *i18n_layout_word(const char *name)
{
    if (!name) return name;
    const char *lang = i18n_active_lang();
    { int i = 0; for (; i < g_dyn_words_n; i++)   
        if (strcmp(g_dyn_words[i].lang, lang) == 0 && strcmp(g_dyn_words[i].alias, name) == 0)
            return g_dyn_words[i].canon; }
    if (strcmp(lang, "fa") == 0) {
        { const i18n_pair_t *p = k_layout_fa; for (; p->key; p++)
            if (strcmp(p->key, name) == 0) return p->val; }
    }
    return name;
}

void i18n_register_layout_value(const char *lang, const char *alias, const char *canonical)
{
    if (!lang || !alias || !canonical || g_dyn_vals_n >= I18N_DYN_VAL_MAX) return;
    g_dyn_vals[g_dyn_vals_n].lang  = lang;
    g_dyn_vals[g_dyn_vals_n].alias = alias;
    g_dyn_vals[g_dyn_vals_n].canon = canonical;
    g_dyn_vals_n++;
}

const char *i18n_layout_value(const char *value)
{
    if (!value) return value;
    const char *lang = i18n_active_lang();
    
    { int i = 0; for (; i < g_dyn_vals_n; i++)
        if ((g_dyn_vals[i].lang[0] == '*' || strcmp(g_dyn_vals[i].lang, lang) == 0) &&
            strcmp(g_dyn_vals[i].alias, value) == 0)
            return g_dyn_vals[i].canon; }
    if (strcmp(lang, "fa") == 0) {
        { const i18n_pair_t *p = k_value_fa; for (; p->key; p++)
            if (strcmp(p->key, value) == 0) return p->val; }
    }
    return value;
}
