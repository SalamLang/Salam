#ifndef SALAM_I18N_I18N_H
#define SALAM_I18N_I18N_H

#include "core/prelude.h"

void        i18n_set_lang(const char *code);

const char *i18n_lang(void);

const char *i18n_tr(const char *msgid);

const char *i18n_layout_word(const char *name);

void i18n_register_layout_word(const char *lang, const char *alias, const char *canonical);

const char *i18n_layout_value(const char *value);

void i18n_register_layout_value(const char *lang, const char *alias, const char *canonical);

#endif /* SALAM_I18N_I18N_H */
