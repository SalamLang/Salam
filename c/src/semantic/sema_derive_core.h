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

#ifndef SALAM_SEMANTIC_SEMA_DERIVE_CORE_H
#define SALAM_SEMANTIC_SEMA_DERIVE_CORE_H

#include "core/prelude.h"
#include "core/sb.h"
#include "semantic/sema_internal.h"

/*
 * The machinery both derive passes are built on: generate Salam source for a
 * function, parse it, and install the result as a synthetic top-level
 * declaration of the program being analysed. A generic instantiation is the
 * same kind of thing, so the C, LLVM, JS and interpreter backends emit what
 * comes out of here without knowing either pass exists.
 *
 * sema_derive_str.c derives `_strof_T` for `println <aggregate>`;
 * sema_derive_json.c derives `_jsonenc_T` / `_jsondec_T` for json.Marshal and
 * json.Unmarshal. They share one registry so a name is claimed once and a
 * self-referential type terminates.
 */

typedef struct derived_t derived_t;

/*
 * Identifier characters for a type's key: `Vector<str>` -> `Vector_str`,
 * `Point[3]` -> `Point_a3`, `geo.Point` -> `geo_Point`. Only has to be
 * injective enough that two different types never collide.
 */
void derive_key_append(sb_t *b, const char *ts);

/* `<prefix><key of t>`, arena-allocated. */
const char *derive_fn_name(sema_t *s, const char *prefix, type_t *t);

/*
 * True when this run has already derived (or is midway through deriving)
 * `name`. The registry is the authority rather than a scope lookup: a user
 * function that happens to be called `_strof_Point` must not be mistaken for
 * one of ours.
 */
bool derive_claimed(sema_t *s, const char *name);

/*
 * Claim `name` before its body is generated. A struct holding a Vector of
 * itself asks for the same function again from inside the emitter, and has to
 * be told it already exists rather than recurse forever.
 */
derived_t *derive_claim(sema_t *s, const char *name);

/* Give the claim back when the body turns out not to be derivable. */
void derive_unclaim(sema_t *s, derived_t *ent);

void derive_set_decl(derived_t *ent, ast_node_t *fn);

/*
 * Parse `text` as a whole program and install every `func` in it as a
 * synthetic top-level of the program being analysed. Returns the one named
 * `want`, or NULL if the text did not parse.
 *
 * `home`, when not NULL, becomes the scope the installed bodies are checked
 * inside - the way a generic instance is checked against the package that
 * declared it. The JSON derive passes std/encoding/json's scope so its
 * generated bodies can call that module's private runtime layer unqualified.
 */
ast_node_t *derive_install_source(sema_t *s, const char *text, const char *want,
                                  scope_t *home);

/* The scope, at or above `from`, that declares `probe` - the way the JSON
 * derive finds std/encoding/json without hardcoding a package name. NULL when
 * no scope on the chain has it. */
scope_t *derive_home_of(scope_t *from, const char *probe);

/* Dump generated sources to stderr. Set by SALAM_DERIVE_DUMP=1. */
bool derive_dump_enabled(void);

#endif /* SALAM_SEMANTIC_SEMA_DERIVE_CORE_H */
