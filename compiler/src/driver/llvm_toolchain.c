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
#include "core/sal_format.h"
#include "driver/llvm_toolchain.h"
#include "llvm/llvm_native.h"
#include "core/sb.h"
#include "i18n/i18n.h"
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#if defined(PATH_MAX)
#define SALAM_PATH_MAX PATH_MAX
#else
#define SALAM_PATH_MAX 4096
#endif

#define LLVM_CLANG "clang-22"
#define LLVM_LLC   "llc-22"
#define LLVM_OPT   "opt-22"
#define LLVM_LLI   "lli-22"

static const char *opt_flag(llvm_opt_level_t l)
{
    switch (l) {
        case LLVM_OPT_O0: return "-O0"; case LLVM_OPT_O1: return "-O1";
        case LLVM_OPT_O2: return "-O2"; case LLVM_OPT_O3: return "-O3";
        case LLVM_OPT_OS: return "-Os"; case LLVM_OPT_OZ: return "-Oz";
        default:          return "-O0";
    }
}

static void to_tool_path(const char *host, char *out, size_t cap)
{
    char abs[SALAM_PATH_MAX];
#if defined(_WIN32)
    if (!_fullpath(abs, host, sizeof abs)) sal_snprintf(abs, sizeof abs, "%s", host);
    
    size_t k = 0;
    if (isalpha((unsigned char)abs[0]) && abs[1] == ':') {
        k = (size_t)sal_snprintf(out, cap, "/mnt/%c", (char)tolower((unsigned char)abs[0]));
        { const char *p = abs + 2; for (; *p && k < cap - 1; p++)
            out[k++] = (*p == '\\') ? '/' : *p; }
        out[k] = 0;
    } else {
        sal_snprintf(out, cap, "%s", abs);
    }
#else
    extern char *realpath(const char *, char *);
    if (host[0] == '/' || !realpath(host, abs)) sal_snprintf(abs, sizeof abs, "%s", host);
    sal_snprintf(out, cap, "%s", abs);
#endif
}

static void emit_tool_cmd(sb_t *s, const codegen_llvm_options_t *opts)
{
    const char *O = opt_flag(opts->opt_level);
    const char *triple = opts->target_triple;
    if (opts->verify_module)
        sb_puts(s, LLVM_OPT " -passes=verify -disable-output \"$IN\"\n");
    switch (opts->output_mode) {
        case LLVM_OUT_ASM:
            sb_puts(s, LLVM_LLC " "); sb_puts(s, O);
            if (triple) { sb_puts(s, " -mtriple="); sb_puts(s, triple); }
            sb_puts(s, " -filetype=asm \"$IN\" -o \"$OUT\"\n");
            break;
        case LLVM_OUT_OBJ:
            sb_puts(s, LLVM_LLC " "); sb_puts(s, O);
            if (triple) { sb_puts(s, " -mtriple="); sb_puts(s, triple); }
            sb_puts(s, " -filetype=obj \"$IN\" -o \"$OUT\"\n");
            break;
        case LLVM_OUT_BITCODE:
            sb_puts(s, LLVM_OPT " "); sb_puts(s, O); sb_puts(s, " \"$IN\" -o \"$OUT\"\n");
            break;
        case LLVM_OUT_EXEC:
            sb_puts(s, LLVM_CLANG " "); sb_puts(s, O);
            if (triple)           { sb_puts(s, " --target="); sb_puts(s, triple); }
            if (opts->debug_info)   sb_puts(s, " -g");
            sb_puts(s, " \"$IN\" -lm -o \"$OUT\"\n");
            break;
        case LLVM_OUT_JIT:
            if (opts->opt_level != LLVM_OPT_O0) {
                sb_puts(s, LLVM_OPT " "); sb_puts(s, O);
                sb_puts(s, " \"$IN\" -o - | " LLVM_LLI " -\n");
            } else {
                sb_puts(s, LLVM_LLI " \"$IN\"\n");
            }
            break;
        case LLVM_OUT_IR:   
            if (!opts->verify_module) sb_puts(s, "true\n");
            break;
    }
}

int salam_llvm_toolchain(logger_t *log, const char *ll_path,
                         const codegen_llvm_options_t *opts)
{
    if (salam_llvm_native_available()) {
        int nrc = salam_llvm_native(log, ll_path, opts);
        if (nrc >= 0) return nrc;
    }

    char in_tp[SALAM_PATH_MAX]; to_tool_path(ll_path, in_tp, sizeof in_tp);
    bool needs_out = opts->output_mode != LLVM_OUT_JIT && opts->output_mode != LLVM_OUT_IR;
    char out_tp[SALAM_PATH_MAX]; out_tp[0] = 0;
    if (needs_out && opts->output_file) to_tool_path(opts->output_file, out_tp, sizeof out_tp);
    
    sb_t script; sb_init(&script);
    sb_puts(&script, "set -e\n");
    sb_puts(&script, "IN='"); sb_puts(&script, in_tp); sb_puts(&script, "'\n");
    if (out_tp[0]) { sb_puts(&script, "OUT='"); sb_puts(&script, out_tp); sb_puts(&script, "'\n"); }
    emit_tool_cmd(&script, opts);
    
    char spath[1024]; sal_snprintf(spath, sizeof spath, "%s.run.sh", ll_path);
    FILE *sf = fopen(spath, "wb");
    if (!sf) { LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), spath); sb_free(&script); return 2; }
    fputs(sb_cstr(&script), sf); fclose(sf);
    sb_free(&script);
    char script_tp[SALAM_PATH_MAX]; to_tool_path(spath, script_tp, sizeof script_tp);
    sb_t cmd; sb_init(&cmd);
#if defined(_WIN32)
    sb_puts(&cmd, "wsl.exe -e bash \""); sb_puts(&cmd, script_tp); sb_puts(&cmd, "\"");
#else
    sb_puts(&cmd, "bash \""); sb_puts(&cmd, script_tp); sb_puts(&cmd, "\"");
#endif
    LOG_I(log, PH_DRIVER, "llvm toolchain: %s", sb_cstr(&cmd));
    int rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
    remove(spath);
    if (rc != 0)
        LOG_E(log, PH_DRIVER, i18n_tr("LLVM toolchain step failed (exit %d)"), rc);
    return rc;
}
