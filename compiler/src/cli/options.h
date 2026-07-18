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

#ifndef SALAM_CLI_OPTIONS_H
#define SALAM_CLI_OPTIONS_H

#include "core/prelude.h"
#include "logger/logger.h"
#include "diag/diag_render.h"

#define SALAM_MAX_INPUTS 64

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
    CMD_VERSION
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
    const char *output;
    const char *cc;
    bool keep_c;
    bool safe;
    bool fmt_check;
    bool fmt_recursive;
    bool fmt_tabs;
    int fmt_indent_width;
    bool debug_info;
    bool asan;
    bool interp;
    const char *stdlib_path;
    char exe_path[512];
    const char *defines[SALAM_MAX_INPUTS];
    int ndefines;
    int llvm_opt_level;
    int llvm_emit;
    bool llvm_verify;
    const char *llvm_target;
    bool llvm_native_cpu;
} options_t;

bool cli_parse(int argc, char **argv, options_t *out);

void cli_print_usage(FILE *out);

#endif /* SALAM_CLI_OPTIONS_H */
