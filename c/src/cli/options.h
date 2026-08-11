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

#ifndef SALAM_CLI_OPTIONS_H
#define SALAM_CLI_OPTIONS_H

#include "core/prelude.h"
#include "core/prof_self.h"
#include "logger/logger.h"
#include "diag/diag_render.h"

/*
 * Also bounds the module import closure in driver_build's work[] - every
 * .salam file reached transitively, not just the files named on the command
 * line. At 64 that was comfortable for a compiler laid out as ~30 flat
 * modules, but the dir-per-stage layout is 150+ files: the closure
 * overflowed and the push sites that bounds-check silently (rather than
 * reporting the limit) dropped modules, so a generated header ended up
 * including one that was never emitted - "salam_mod_encoding.h: No such
 * file or directory", with nothing pointing at the real cause.
 */
#define SALAM_MAX_INPUTS 1024

typedef enum {
    CMD_INSPECT = 0,
    CMD_BUILD,
    CMD_OBJ,
    CMD_LLVM,
    CMD_JS,
    CMD_RUN,
    CMD_LAYOUT_BUILD,
    CMD_WEB,
    CMD_DEBUG,
    CMD_MEMCHECK,
    CMD_NEW,
    CMD_FMT,
    CMD_REPL,
    CMD_REPL_LAYOUT,
    CMD_HELP,
    CMD_VERSION,
    CMD_SERVE,
    CMD_DOC,
    CMD_UNKNOWN
} cli_command_t;

typedef struct {
    cli_command_t command;
    const char *input;
    const char *lang;
    log_level_t log_level;
    bool emit_tokens_xml;
    bool emit_ast_xml;
    bool emit_symbol_xml;
    const char *xml_out;
    int color;
    diag_style_t diag_style;
    diag_format_t diag_format;
    const char *new_name;
    bool inline_mode;
    bool split;
    const char *inputs[SALAM_MAX_INPUTS];
    int input_count;
    const char *run_args[SALAM_MAX_INPUTS];
    int run_args_count;
    const char *output;
    const char *cc;
    /* Whether `cc` came from an explicit --cc=. The default is the literal
     * string "tcc", so without this the auto-resolution ladder in build.c
     * could not tell "nobody asked" from "the user asked for tcc" and
     * silently replaced an explicit --cc=tcc with whatever clang/gcc it
     * found first - which is how every `--cc=tcc` test job (opencv, db)
     * quietly started building with a different compiler. */
    bool cc_explicit;
    /* Backend selection for `salam build`/`obj`: "auto" (default - LLVM when
     * this binary has it compiled in, C otherwise), "llvm", or "c". Set to
     * "c" implicitly by an explicit --cc=, which only the C backend uses. */
    const char *backend;
    bool keep_c;
    bool force;
    bool safe;
    bool fmt_check;
    bool fmt_recursive;
    bool fmt_tabs;
    int fmt_indent_width;
    bool fmt_fix_order;
    bool fmt_minify;
    bool debug_info;
    bool asan;
    /* --lto: whole-program LTO on the final link. Opt-in, not implied by an
     * optimizing build: the default path compiles each module to its own .o
     * and reuses the unchanged ones, which -flto defeats (every object is
     * bitcode, so the link is one serial LTRANS pass over all of them). On
     * Windows the toolchain's lto-wrapper falls back to serial LTRANS no
     * matter what -flto=N asks for, and mingw's lto1 is where this host's
     * `internal compiler error: original not compressed with zstd` comes
     * from - so the fast path stays the default and LTO is a choice. */
    bool lto;
    bool interp;
    /* --timeout=MS: the tree-walking interpreter's wall-clock deadline for
     * `run`/`exec`/`jit`. 0 = unset, in which case SALAM_INTERP_TIMEOUT_MS
     * is consulted and interp.c's own default applies after that. */
    int timeout_ms;
    const char *stdlib_path;
    char exe_path[512];
    const char *defines[SALAM_MAX_INPUTS];
    int ndefines;
    /* --export=Fn:CName, repeatable: keep top-level pub func Fn alive through
     * dead-code elimination and emit it under the literal C name CName rather
     * than the mangled one, so a host with no Salam entry point can link
     * against a hand-picked API. Applies to the entry file only - matching by
     * name across every transitively imported module would rename unrelated
     * same-named functions too. */
    const char *exports[SALAM_MAX_INPUTS];
    int nexports;
    /* --libpath=DIR, repeatable: extra library search directories for the
     * link step, for archives not installed under the usual prefix. */
    const char *lib_paths[SALAM_MAX_INPUTS];
    int nlibpath;
    int llvm_opt_level;
    int llvm_emit;
    bool llvm_verify;
    const char *llvm_target;
    bool llvm_native_cpu;
    bool no_js_minify_names;
    bool no_minify;
    bool version_short;
    const char *serve_host;
    int serve_port;
    /* --time-report[=table|json]: where the compiler spent its own time. */
    bool time_report;
    int time_report_fmt; /* PROF_FMT_TABLE | PROF_FMT_JSON */
    /* --time-trace[=FILE]: Chrome Trace Event JSON of the same run. */
    const char *time_trace;
} options_t;

#define OPTIONS_INIT_NO_JS_MINIFY_NAMES 0
#define OPTIONS_INIT_NO_MINIFY 0

bool cli_parse(int argc, char **argv, options_t *out);

void cli_print_usage(FILE *out);

#endif /* SALAM_CLI_OPTIONS_H */
