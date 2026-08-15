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

#ifndef SALAM_SEMANTIC_SEMA_DERIVE_JSON_H
#define SALAM_SEMANTIC_SEMA_DERIVE_JSON_H

#include "core/prelude.h"
#include "semantic/sema_internal.h"

/*
 * Derive a JSON codec for `t` and hand back the function's name, or NULL when
 * the type has no JSON form (the caller then reports the error).
 *
 *   sema_derive_json_enc  ->  func _jsonenc_T(_v: T): str
 *   sema_derive_json_dec  ->  func _jsondec_T(_s: str, _i: int, _o &: T,
 *                                             _e &: str, _ep &: int,
 *                                             _st: bool): int
 *
 * The decoder returns the position just past the value it consumed, or -1
 * with `_e`/`_ep` filled in. `_st` is the strict flag: it decides what an
 * unknown key, a missing key and a repeated key mean, and is threaded through
 * every nested call so one Unmarshal has one policy throughout.
 *
 * `home` is std/encoding/json's scope. The generated bodies are installed to
 * be checked inside it, so they reach that module's private runtime layer
 * (_dqs, _dgstr, _derr and the rest) without qualification and without any of
 * it having to be public.
 */
const char *sema_derive_json_enc(sema_t *s, type_t *t, scope_t *home,
                                 const src_span_t *span);
const char *sema_derive_json_dec(sema_t *s, type_t *t, scope_t *home,
                                 const src_span_t *span);

#endif /* SALAM_SEMANTIC_SEMA_DERIVE_JSON_H */
