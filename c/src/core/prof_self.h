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

#ifndef SALAM_CORE_PROF_SELF_H
#define SALAM_CORE_PROF_SELF_H

#include "core/prelude.h"

/* Self-profiling for the compiler itself: `salam build --time-report`.
 *
 * Every entry point is a no-op until prof_self_enable() has been called, so
 * instrumentation can sit in the pipeline unconditionally. The cost when off
 * is one predictable branch on a file-static bool. */

typedef enum {
    TP_DRIVER = 0,
    TP_SOURCE,
    TP_LEXER,
    TP_PARSER,
    TP_SEMANTIC,
    TP_CODEGEN,
    TP_LLVM,
    TP_JSGEN,
    TP_WRITE,
    TP_CC,
    TP_LINK,
    TP__COUNT
} timing_phase_t;

typedef enum {
    TC_TOKENS = 0,
    TC_AST_NODES,
    TC_SYMBOLS,
    TC_FUNCS_EMITTED,
    TC_MODULES_BUILT,
    TC_MODULES_CACHED,
    TC_ARENA_BYTES,
    TC_ARENA_BLOCKS,
    TC__COUNT
} timing_counter_t;

#define PROF_FMT_TABLE 0
#define PROF_FMT_JSON 1

/* `trace_events` also records every scope as a Chrome Trace Event, which
 * prof_self_write_trace() can dump. Keep it off for a plain --time-report;
 * the event list is unbounded. */
void prof_self_enable(bool trace_events);

bool prof_self_on(void);

/* Opens a timing scope. The returned token must be handed back to
 * prof_self_end(). Nesting is supported: an inner scope's elapsed time is
 * subtracted from the enclosing scope's "self" time but stays in its "total".
 * `label` is borrowed, not copied, so it must outlive the report (source
 * paths out of the arena qualify). It may be NULL. */
uint64_t prof_self_begin(timing_phase_t ph, const char *label);

void prof_self_end(timing_phase_t ph, uint64_t token);

void prof_self_count(timing_counter_t c, uint64_t n);

/* `fmt` is PROF_FMT_TABLE or PROF_FMT_JSON. */
void prof_self_report(FILE *out, int fmt);

/* Chrome Trace Event JSON, readable by chrome://tracing, Perfetto and
 * speedscope. Returns false when the file cannot be written. */
bool prof_self_write_trace(const char *path);

void prof_self_shutdown(void);

const char *prof_self_phase_name(timing_phase_t ph);

#define PROF_SCOPE_BEGIN(ph, label) uint64_t _pt_##ph = prof_self_begin((ph), (label))
#define PROF_SCOPE_END(ph) prof_self_end((ph), _pt_##ph)

#endif /* SALAM_CORE_PROF_SELF_H */
