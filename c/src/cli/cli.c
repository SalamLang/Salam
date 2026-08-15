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

#include "core/prelude.h"
#include "cli/cli_internal.h"
#include "i18n/i18n.h"

static void cli_set_defaults(options_t *out)
{
    /*
     * Zero first, then set the non-zero defaults below. driver_main declares
     * its options_t as a plain stack local, so anything this function forgets
     * to assign is read as whatever was on the stack.
     *
     * That is not hypothetical: --lto, --timeout and --export added fields
     * here and none of them were listed below, so `lto` and `timeout_ms` took
     * random values on every run, and `nexports` - a count driving a loop
     * over `exports[]` - sent driver_build through strchr() on garbage
     * pointers. Every `salam build` and `salam obj` segfaulted, including on
     * hello-world. Assigning field-by-field means each new option silently
     * inherits that bug until someone remembers; a memset cannot forget.
     */
    memset(out, 0, sizeof *out);
    out->input = NULL;
    out->lang = "en";
    out->log_level = LOG_INFO;
    out->emit_tokens_xml = false;
    out->emit_ast_xml = false;
    out->emit_symbol_xml = false;
    out->xml_out = NULL;
    out->color = -1;
    out->diag_style = DIAG_STYLE_RUST;
    out->diag_format = DIAG_FORMAT_HUMAN;
    out->command = CMD_INSPECT;
    out->new_name = NULL;
    out->inline_mode = false;
    out->split = false;
    out->input_count = 0;
    out->run_args_count = 0;
    out->output = NULL;
    out->cc = "tcc";
    out->cc_explicit = false;
    out->backend = "auto";
    out->keep_c = false;
    out->force = false;
    out->safe = true;
    out->fmt_check = false;
    out->fmt_recursive = false;
    out->fmt_tabs = false;
    out->fmt_indent_width = 4;
    out->fmt_fix_order = false;
    out->fmt_minify = false;
    /* Also covered by the memset above; spelled out so the intended value of
       each is visible next to its siblings. timeout_ms 0 means "unset" - see
       driver.c's `opt->timeout_ms > 0` test. */
    out->lto = false;
    out->timeout_ms = 0;
    out->nexports = 0;
    out->debug_info = false;
    out->asan = false;
    out->interp = false;
    out->stdlib_path = NULL;
    out->exe_path[0] = '\0';
    out->ndefines = 0;
    out->nconsts = 0;
    out->nlibpath = 0;
    out->llvm_opt_level = 0;
    out->llvm_emit = 0;
    out->llvm_verify = false;
    out->llvm_target = NULL;
    out->llvm_native_cpu = false;
    out->no_js_minify_names = OPTIONS_INIT_NO_JS_MINIFY_NAMES;
    out->no_minify = OPTIONS_INIT_NO_MINIFY;
    out->version_short = false;
    out->serve_host = NULL;
    out->serve_port = -1;
    out->time_report = false;
    out->time_report_fmt = PROF_FMT_TABLE;
    out->time_trace = NULL;
}

bool cli_parse(int argc, char **argv, options_t *out)
{
    cli_set_defaults(out);
    int start = cli_dispatch_command(argc, argv, out);
    if (out->command == CMD_UNKNOWN) {
        fprintf(stderr, i18n_tr("salam: unknown command '%s'\n\n"), argv[1]);
        cli_print_usage(stderr);
        return false;
    }
    if (!cli_parse_options(argc, argv, start, out)) return false;
    if (out->safe && out->ndefines < SALAM_MAX_INPUTS)
        out->defines[out->ndefines++] = "SALAM_SAFE";
    return true;
}
