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

#ifndef SALAM_SEMANTIC_SEMA_DERIVE_INTERNAL_H
#define SALAM_SEMANTIC_SEMA_DERIVE_INTERNAL_H

#include "core/prelude.h"
#include "core/sb.h"
#include "semantic/sema_derive_str.h"

/* Locals the generated bodies use. Underscore-prefixed so they never collide
 * with a field or a package name, and so the unused-name rules let them be. */
#define DV_PARAM "_v"
#define DV_BUF "_sb"

/*
 * How `t` is written in source, resolvable from the printing package's global
 * scope - "i32", "Vector<str>", "geo.Point", "Point[3]". NULL when the type
 * has no such spelling (a struct another package keeps private, a function
 * type, a variant) and the print has to stay an error.
 */
const char *derive_type_spelling(sema_t *s, type_t *t);

/*
 * The full `func <fname>(_v: T): str: ... end` text for `t`, or NULL when some
 * part of it is not renderable. Nested aggregates are derived on the way
 * through, so this may install further functions before it returns.
 */
const char *derive_fn_source(sema_t *s, type_t *t, const char *fname,
                             const src_span_t *span);

/*
 * Statements appending the rendering of the Salam expression `expr` (of type
 * `t`) to DV_BUF.
 */
void derive_append(sema_t *s, sb_t *b, type_t *t, const char *expr,
                   const src_span_t *span);

/*
 * Shape questions both derive passes ask about a struct. Recognised by shape
 * rather than by name, so std/collections' Vector, Deque and LinkedList and
 * anything a user writes the same way are all treated as the sequence or the
 * map they are.
 */
func_sig_t *derive_method_sig(symbol_t *sym, const char *name, size_t nparams);
bool derive_struct_seq(symbol_t *sym, type_t **elem, const char **count);
bool derive_struct_map(symbol_t *sym, type_t **key, type_t **val);

/* bool, char, uchar or a number: everything with a direct JSON spelling and
 * a direct print spelling. */
bool derive_is_plain(const type_t *t);

/* What a struct calls itself in generated text: `Box<i32>` rather than the
 * internal `Box_i32` a generic instance is named. */
const char *derive_display_name(sema_t *s, symbol_t *sym);

#endif /* SALAM_SEMANTIC_SEMA_DERIVE_INTERNAL_H */
