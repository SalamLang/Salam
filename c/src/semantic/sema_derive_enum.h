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

#ifndef SALAM_SEMANTIC_SEMA_DERIVE_ENUM_H
#define SALAM_SEMANTIC_SEMA_DERIVE_ENUM_H

#include "core/prelude.h"
#include "semantic/sema_internal.h"

/*
 * The built-in enum reflection operations, each derived once per enum type
 * the same way sema_derive_json.c derives JSON encode/decode: generate
 * Salam source, parse it, install it as a synthetic top-level declaration.
 * Every backend gets the result for free - no per-backend codegen exists
 * for any of these three.
 *
 *   sema_derive_enum_name        -> func _enum_name_T(_v: T): str
 *   sema_derive_enum_fromstring  -> func _enum_fromstring_T(_s: str, _ok &: bool): T
 *   sema_derive_enum_names       -> func _enum_names_T(): Vector<str>
 *
 * `t` must be a TY_ENUM type with a non-empty member list; returns the
 * installed function's name, or NULL if `t` isn't a derivable enum (empty
 * enum - sema already rejects that at the declaration itself, so this is
 * just defensive).
 */
const char *sema_derive_enum_name(sema_t *s, type_t *t, const src_span_t *span);

const char *sema_derive_enum_fromstring(sema_t *s, type_t *t, const src_span_t *span);

const char *sema_derive_enum_names(sema_t *s, type_t *t, const src_span_t *span);

#endif /* SALAM_SEMANTIC_SEMA_DERIVE_ENUM_H */
