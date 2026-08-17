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

#ifndef SALAM_SEMANTIC_SEMA_DERIVE_STR_H
#define SALAM_SEMANTIC_SEMA_DERIVE_STR_H

#include "core/prelude.h"
#include "semantic/sema_internal.h"

/*
 * Derive `func _strof_<T>(v: T): str` for an aggregate the user asked to
 * print, and hand back its name. NULL when the type has no printable form
 * (the caller then reports the ordinary "cannot print" error).
 *
 * The function is generated as Salam source, parsed, and installed in the
 * current program as a synthetic top-level declaration - the same treatment
 * a generic instantiation gets, so every backend emits it without knowing
 * this pass exists.
 */
const char *sema_derive_stringify(sema_t *s, type_t *t, const src_span_t *span);

/* True while a derived body is being checked. Struct field privacy does not
 * apply to it: `println p` shows every field the struct has, the way the
 * language's own diagnostics and the debugger do. */
bool sema_is_derived_decl(sema_t *s, ast_node_t *d);

#endif /* SALAM_SEMANTIC_SEMA_DERIVE_STR_H */
