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

#ifndef SALAM_FMT_FMT_ORDER_H
#define SALAM_FMT_FMT_ORDER_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/sb.h"
#include "core/vec.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"

/*
 * Rewrites `src` so its top-level declarations satisfy the ordering rules
 * enforced by sema_check_toplevel_order() (E083-E088): imports/extern
 * prototypes -> const -> var -> struct/enum/interface/impl/type-alias ->
 * func, non-pub funcs before pub funcs. It is a pure reordering: every moved
 * declaration's exact original text (including its 'pub'/etc. modifiers,
 * any immediately-preceding '@lang' annotations, and immediately-preceding
 * '//' comment lines) is relocated verbatim, never rewritten.
 *
 * Declarations the fixer cannot safely relocate on its own (anything inside
 * an 'extern: ... end' block, anything that would require moving across a
 * top-level conditional-compilation 'if' block, or anything preceded by
 * trivia this scan can't confidently classify, such as a block comment) are
 * left untouched; any ordering problem that remains after the fix is
 * reported in `notes` (as human-readable, already-localized diagnostic
 * strings) for the user to resolve by hand.
 *
 * Returns false only if `src` itself fails to parse (in which case `out`
 * is left holding an unmodified copy of `src` and `notes` is left empty) -
 * callers should treat that the same as "nothing to reorder" and fall back
 * to formatting the original source. `notes` may be NULL if the caller
 * doesn't want the leftover-problem report.
 */
bool fmt_reorder_toplevel(arena_t *a, logger_t *log, const langpack_t *pack,
                          const source_file_t *src, const char *file, sb_t *out,
                          vec_t *notes);

#endif /* SALAM_FMT_FMT_ORDER_H */
