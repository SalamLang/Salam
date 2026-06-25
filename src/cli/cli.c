#include "core/prelude.h"
#include "cli/cli_internal.h"

static void cli_set_defaults(options_t *out)
{
    out->input = NULL;
    out->lang = "en";
    out->log_level = LOG_INFO;
    out->emit_tokens_xml = false;
    out->emit_ast_xml = false;
    out->emit_symbol_xml = false;
    out->xml_out = NULL;
    out->color       = -1;
    out->diag_style  = DIAG_STYLE_RUST;
    out->diag_format = DIAG_FORMAT_HUMAN;
    out->command = CMD_INSPECT;
    out->new_name = NULL;
    out->inline_mode = false;
    out->input_count = 0;
    out->output = NULL;
    out->cc = "tcc";
    out->keep_c = false;
    out->safe = false;
    out->fmt_check = false;
    out->fmt_recursive = false;
    out->fmt_tabs = false;
    out->fmt_indent_width = 4;
    out->debug_info = false;
    out->asan = false;
    out->interp = false;
    out->stdlib_path = NULL;
    out->exe_path[0] = '\0';
    out->ndefines = 0;
    out->llvm_opt_level = 0;
    out->llvm_emit = 0;
    out->llvm_verify = false;
    out->llvm_target = NULL;
}

bool cli_parse(int argc, char **argv, options_t *out)
{
    cli_set_defaults(out);
    int start = cli_dispatch_command(argc, argv, out);
    return cli_parse_options(argc, argv, start, out);
}
