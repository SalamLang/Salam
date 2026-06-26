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
    CMD_INSPECT = 0,   /* salam <file>          inspect / emit XML (default)   */
    CMD_BUILD,         /* salam build <files>   compile to an executable       */
    CMD_OBJ,           /* salam obj   <files>   compile to an object file      */
    CMD_LLVM,          /* salam llvm  <file>    emit LLVM IR (.ll)             */
    CMD_RUN,           /* salam run [file]      build (auto-detect) + run       */
    CMD_LAYOUT_BUILD,  /* salam layout build    layout DSL -> HTML/CSS/JS      */
    CMD_DEBUG,         /* salam debug <file>    build -g + launch GDB/LLDB     */
    CMD_MEMCHECK,      /* salam memcheck <file> build ASAN + run memchecker    */
    CMD_NEW,           /* salam new [name]      scaffold a project directory   */
    CMD_FMT,           /* salam fmt [files|dirs] format .salam source in place  */
    CMD_REPL,          /* salam cli             general REPL                   */
    CMD_REPL_LAYOUT,   /* salam layout          layout REPL                    */
    CMD_HELP,          /* salam help            print usage                    */
    CMD_VERSION        /* salam version         print version                  */
} cli_command_t;

typedef struct {
    cli_command_t command;         /* the sub-command (CMD_INSPECT default)     */
    const char  *input;            /* positional: source file (required) */
    const char  *lang;             /* --lang=en (default "en")           */
    log_level_t  log_level;        /* --log-level= (default INFO)        */
    bool         emit_tokens_xml;  /* --emit-tokens-xml                  */
    bool         emit_ast_xml;     /* --emit-ast-xml (decorated)         */
    bool         emit_symbol_xml;  /* --emit-symbol-xml                  */
    const char  *xml_out;          /* --xml-out=FILE (NULL => stdout)    */
    int          color;            /* --color / --no-color               */
    diag_style_t  diag_style;     /* --error-style=rust|gcc|clang       */
    diag_format_t diag_format;    /* --error-format=human|json|xml      */
    const char  *new_name;         /* CMD_NEW project name (NULL => prompt) */
    bool         inline_mode;      /* --inline (layout)                  */
    const char  *inputs[SALAM_MAX_INPUTS];
    int          input_count;
    const char  *output;           /* --output (NULL => <module>.exe)    */
    const char  *cc;               /* --cc (default "tcc")               */
    bool         keep_c;           /* --keep-c                           */
    bool         safe;             /* --safe (runtime bounds checks)     */
    bool         fmt_check;        /* fmt --check: report, don't rewrite */
    bool         fmt_recursive;    /* fmt -r/--recursive: walk directories */
    bool         fmt_tabs;         /* fmt --tabs/--indent=tab: indent with tabs */
    int          fmt_indent_width; /* fmt --indent=N: spaces per level (default 4) */
    bool         debug_info;       /* -g / --debug-info: DWARF symbols   */
    bool         asan;             /* --asan: AddressSanitizer           */
    bool         interp;           /* run via the tree-walking interpreter */
    const char  *stdlib_path;      /* --stdlib-path=DIR (root for std/)  */
    char         exe_path[512];    /* filled by driver_build on success  */
    const char  *defines[SALAM_MAX_INPUTS];
    int          ndefines;
    int          llvm_opt_level;
    int          llvm_emit;
    bool         llvm_verify;       /* --verify-ir                            */
    const char  *llvm_target;       /* --target=TRIPLE (NULL = host)          */
} options_t;

bool cli_parse(int argc, char **argv, options_t *out);

void cli_print_usage(FILE *out);

#endif /* SALAM_CLI_OPTIONS_H */
