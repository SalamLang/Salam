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

#include "core/prelude.h"
#include "cli/cli_internal.h"
#include "i18n/i18n.h"

const char *cli_opt_value(const char *arg, const char *key)
{
    size_t klen = strlen(key);
    if (strncmp(arg, key, klen) == 0 && arg[klen] == '=') {
        return arg + klen + 1;
    }
    return NULL;
}

static bool cli_positional(const char *arg, options_t *out)
{
    switch (out->command) {
    case CMD_NEW:
        if (out->new_name != NULL) {
            fprintf(stderr,
                    i18n_tr("salam: 'new' takes a single project name "
                            "('%s' after '%s')\n"),
                    arg, out->new_name);
            return false;
        }
        out->new_name = arg;
        break;
    case CMD_BUILD:
    case CMD_OBJ:
    case CMD_LLVM:
    case CMD_JS:
    case CMD_RUN:
    case CMD_LAYOUT_BUILD:
    case CMD_WEB:
    case CMD_DEBUG:
    case CMD_MEMCHECK:
    case CMD_FMT:
        if (out->input_count >= SALAM_MAX_INPUTS) {
            fprintf(stderr, "%s", i18n_tr("salam: too many input files\n"));
            return false;
        }
        out->inputs[out->input_count++] = arg;
        if (!out->input) out->input = arg;
        break;
    default:
        if (out->input != NULL) {
            fprintf(stderr,
                    i18n_tr("salam: multiple input files require the 'build' command "
                            "('%s' after '%s')\n"),
                    arg, out->input);
            return false;
        }
        out->input = arg;
        break;
    }
    return true;
}

bool cli_parse_options(int argc, char **argv, int start, options_t *out)
{
    {
        int i = start;
        for (; i < argc; i++) {
            const char *arg = argv[i];
            const char *val;
            if (strcmp(arg, "--keep-c") == 0) {
                out->keep_c = true;

            } else if (strcmp(arg, "--short") == 0 && out->command == CMD_VERSION) {
                out->version_short = true;
            } else if (strcmp(arg, "-O0") == 0) {
                out->llvm_opt_level = 0;
            } else if (strcmp(arg, "-O1") == 0) {
                out->llvm_opt_level = 1;
            } else if (strcmp(arg, "-O2") == 0) {
                out->llvm_opt_level = 2;
            } else if (strcmp(arg, "-O3") == 0) {
                out->llvm_opt_level = 3;
            } else if (strcmp(arg, "-Os") == 0) {
                out->llvm_opt_level = 4;
            } else if (strcmp(arg, "-Oz") == 0) {
                out->llvm_opt_level = 5;
            } else if (strcmp(arg, "--emit-llvm") == 0) {
                out->llvm_emit = 0;
            } else if (strcmp(arg, "--emit-asm") == 0) {
                out->llvm_emit = 1;
            } else if (strcmp(arg, "--emit-obj") == 0 || strcmp(arg, "-c") == 0) {
                out->llvm_emit = 2;
            } else if (strcmp(arg, "--emit-bitcode") == 0 || strcmp(arg, "-flto") == 0) {
                out->llvm_emit = 3;
            } else if (strcmp(arg, "--exec") == 0 || strcmp(arg, "--emit-exec") == 0) {
                out->llvm_emit = 4;
            } else if (strcmp(arg, "--jit") == 0 && out->command == CMD_LLVM) {
                out->llvm_emit = 5;
            } else if (strcmp(arg, "--verify-ir") == 0) {
                out->llvm_verify = true;
            } else if (strcmp(arg, "--native-cpu") == 0 ||
                       strcmp(arg, "-march=native") == 0) {
                out->llvm_native_cpu = true;
            } else if ((val = cli_opt_value(arg, "--target")) != NULL) {
                out->llvm_target = val;
            } else if (strcmp(arg, "--safe") == 0) {
                out->safe = true;
            } else if (strcmp(arg, "--check") == 0) {
                out->fmt_check = true;
            } else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--recursive") == 0) {
                out->fmt_recursive = true;
            } else if (strcmp(arg, "--tabs") == 0) {
                out->fmt_tabs = true;
            } else if (strcmp(arg, "--spaces") == 0) {
                out->fmt_tabs = false;
            } else if ((val = cli_opt_value(arg, "--indent")) != NULL) {
                if (strcmp(val, "tab") == 0 || strcmp(val, "tabs") == 0) {
                    out->fmt_tabs = true;
                } else {
                    char *endp = NULL;
                    long n = strtol(val, &endp, 10);
                    if (endp == val || *endp != '\0' || n < 1 || n > 16) {
                        fprintf(stderr,
                                i18n_tr("salam: invalid --indent value '%s' "
                                        "(use 'tab' or a width 1-16)\n"),
                                val);
                        return false;
                    }
                    out->fmt_tabs = false;
                    out->fmt_indent_width = (int)n;
                }
            } else if (strcmp(arg, "-g") == 0 || strcmp(arg, "--debug-info") == 0) {
                out->debug_info = true;
            } else if (strcmp(arg, "--asan") == 0) {
                out->asan = true;
            } else if (strcmp(arg, "--interp") == 0 || strcmp(arg, "--jit") == 0) {
                out->interp = true;
            } else if (strcmp(arg, "--inline") == 0) {
                out->inline_mode = true;
            } else if (strcmp(arg, "--split") == 0) {
                out->split = true;
            } else if (strcmp(arg, "--no-js-minify-names") == 0) {
                out->no_js_minify_names = true;
            } else if ((val = cli_opt_value(arg, "--output")) != NULL) {
                out->output = val;
            } else if (strcmp(arg, "-o") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "%s",
                            i18n_tr("salam: -o requires a file argument\n"));
                    return false;
                }
                out->output = argv[++i];
            } else if ((val = cli_opt_value(arg, "--cc")) != NULL) {
                out->cc = val;
            } else if ((val = cli_opt_value(arg, "--stdlib-path")) != NULL) {
                out->stdlib_path = val;
            } else if ((val = cli_opt_value(arg, "--define")) != NULL) {
                if (out->ndefines >= SALAM_MAX_INPUTS) {
                    fprintf(stderr, "%s", i18n_tr("salam: too many macro definitions\n"));
                    return false;
                }
                out->defines[out->ndefines++] = val;
            } else if (strncmp(arg, "-D", 2) == 0 && arg[2]) {
                if (out->ndefines >= SALAM_MAX_INPUTS) {
                    fprintf(stderr, "%s", i18n_tr("salam: too many macro definitions\n"));
                    return false;
                }
                out->defines[out->ndefines++] = arg + 2;
            } else if (strcmp(arg, "--emit-tokens-xml") == 0) {
                out->emit_tokens_xml = true;
            } else if (strcmp(arg, "--emit-ast-xml") == 0) {
                out->emit_ast_xml = true;
            } else if (strcmp(arg, "--emit-symbol-xml") == 0 ||
                       strcmp(arg, "--emit-symbols-xml") == 0) {
                out->emit_symbol_xml = true;
            } else if (strcmp(arg, "--color") == 0) {
                out->color = 1;
            } else if (strcmp(arg, "--no-color") == 0) {
                out->color = 0;
            } else if ((val = cli_opt_value(arg, "--error-style")) != NULL) {
                if (!diag_style_from_string(val, &out->diag_style)) {
                    fprintf(stderr,
                            i18n_tr("salam: invalid error style '%s' (rust|gcc|clang)\n"),
                            val);
                    return false;
                }
            } else if ((val = cli_opt_value(arg, "--error-format")) != NULL) {
                if (!diag_format_from_string(val, &out->diag_format)) {
                    fprintf(
                        stderr,
                        i18n_tr("salam: invalid error format '%s' (human|json|xml)\n"),
                        val);
                    return false;
                }
            } else if ((val = cli_opt_value(arg, "--log-level")) != NULL) {
                if (!log_level_from_string(val, &out->log_level)) {
                    fprintf(stderr, i18n_tr("salam: invalid log level '%s'\n"), val);
                    return false;
                }
            } else if ((val = cli_opt_value(arg, "--lang")) != NULL) {
                out->lang = val;
            } else if ((val = cli_opt_value(arg, "--xml-out")) != NULL) {
                out->xml_out = val;
            } else if (arg[0] == '-' && arg[1] != '\0') {
                fprintf(stderr, i18n_tr("salam: unknown option '%s'\n"), arg);
                return false;
            } else {
                if (!cli_positional(arg, out)) return false;
            }
        }
    }
    return true;
}
