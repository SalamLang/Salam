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
#include "semantic/sema.h"
#include "core/sb.h"
#include "i18n/i18n.h"
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#if defined(PATH_MAX)
#  define SALAM_PATH_MAX PATH_MAX
#else
#  define SALAM_PATH_MAX 4096
#endif

#define LLVM_CLANG "clang-22"
#define LLVM_LLC "llc-22"
#define LLVM_OPT "opt-22"
#define LLVM_LLI "lli-22"

static const char *opt_flag(llvm_opt_level_t l)
{
    switch (l) {
    case LLVM_OPT_O0:
        return "-O0";
    case LLVM_OPT_O1:
        return "-O1";
    case LLVM_OPT_O2:
        return "-O2";
    case LLVM_OPT_O3:
        return "-O3";
    case LLVM_OPT_OS:
        return "-Os";
    case LLVM_OPT_OZ:
        return "-Oz";
    default:
        return "-O0";
    }
}

static void to_tool_path(const char *host, char *out, size_t cap)
{
    char abs[SALAM_PATH_MAX];
#if defined(_WIN32)
    if (!_fullpath(abs, host, sizeof abs)) sal_snprintf(abs, sizeof abs, "%s", host);

    size_t k = 0;
    if (isalpha((unsigned char)abs[0]) && abs[1] == ':') {
        k = (size_t)sal_snprintf(out, cap, "/mnt/%c",
                                 (char)tolower((unsigned char)abs[0]));
        {
            const char *p = abs + 2;
            for (; *p && k < cap - 1; p++)
                out[k++] = (*p == '\\') ? '/' : *p;
        }
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

static bool triple_is_windows(const char *t)
{
    return t && (strstr(t, "windows") || strstr(t, "mingw") || strstr(t, "win32"));
}

static bool triple_is_wasm(const char *t)
{
    return t && strstr(t, "wasm");
}

static void emit_llvm_link(sb_t *s, const char *spec, const char *kind, bool windows)
{
    bool framework = kind && strcmp(kind, "framework") == 0;
    bool is_static = kind && strcmp(kind, "static") == 0;
    if (framework) {
        sb_puts(s, " -framework ");
        sb_put_shell_arg(s, spec);
        return;
    }
    if (spec[0] == '-' || strpbrk(spec, "/\\.") != NULL) {
        sb_putc(s, ' ');
        sb_put_shell_arg(s, spec);
        return;
    }
    if (is_static && !windows) {
        sb_puts(s, " -Wl,-Bstatic -l");
        sb_put_shell_arg(s, spec);
        sb_puts(s, " -Wl,-Bdynamic");
        return;
    }
    sb_puts(s, " -l");
    sb_put_shell_arg(s, spec);
}

static const char *resolve_tool(const char *bundled, const char *fallback, char *buf,
                                size_t cap)
{
    char raw[SALAM_PATH_MAX];
    if (salam_find_bundled_tool(bundled, raw, sizeof raw)) {
        to_tool_path(raw, buf, cap);
        return buf;
    }
    return fallback;
}

static void emit_tool(sb_t *s, const char *tool)
{
    sb_putc(s, '"');
    sb_puts(s, tool);
    sb_putc(s, '"');
}

static void emit_tool_cmd(sb_t *s, const codegen_llvm_options_t *opts)
{
    const char *O = opt_flag(opts->opt_level);
    const char *triple = opts->target_triple;
    static char cbuf[SALAM_PATH_MAX], lbuf[SALAM_PATH_MAX], obuf[SALAM_PATH_MAX],
        ibuf[SALAM_PATH_MAX];
    const char *CLANG = resolve_tool("clang", LLVM_CLANG, cbuf, sizeof cbuf);
    const char *LLC = resolve_tool("llc", LLVM_LLC, lbuf, sizeof lbuf);
    const char *OPTT = resolve_tool("opt", LLVM_OPT, obuf, sizeof obuf);
    const char *LLI = resolve_tool("lli", LLVM_LLI, ibuf, sizeof ibuf);
    if (opts->verify_module) {
        emit_tool(s, OPTT);
        sb_puts(s, " -passes=verify -disable-output \"$IN\"\n");
    }
    switch (opts->output_mode) {
    case LLVM_OUT_ASM:
        emit_tool(s, LLC);
        sb_putc(s, ' ');
        sb_puts(s, O);
        if (triple) {
            sb_puts(s, " -mtriple=");
            sb_puts(s, triple);
        } else if (opts->native_cpu) {
            sb_puts(s, " -mcpu=native");
        }
        sb_puts(s, " -filetype=asm \"$IN\" -o \"$OUT\"\n");
        break;
    case LLVM_OUT_OBJ:
        emit_tool(s, LLC);
        sb_putc(s, ' ');
        sb_puts(s, O);
        if (triple) {
            sb_puts(s, " -mtriple=");
            sb_puts(s, triple);
        } else if (opts->native_cpu) {
            sb_puts(s, " -mcpu=native");
        }
        sb_puts(s, " -filetype=obj \"$IN\" -o \"$OUT\"\n");
        break;
    case LLVM_OUT_BITCODE:
        emit_tool(s, OPTT);
        sb_putc(s, ' ');
        sb_puts(s, O);
        sb_puts(s, " \"$IN\" -o \"$OUT\"\n");
        break;
    case LLVM_OUT_EXEC: {
        bool win = triple_is_windows(triple);
        bool wasm = triple_is_wasm(triple);
        emit_tool(s, CLANG);
        sb_putc(s, ' ');
        sb_puts(s, O);
        if (triple) {
            sb_puts(s, " --target=");
            sb_puts(s, triple);
        } else if (opts->native_cpu) {
            sb_puts(s, " -march=native");
        }
        if (opts->debug_info) sb_puts(s, " -g");
        if (opts->sysroot && opts->sysroot[0]) sb_puts(s, " --sysroot=\"$SYSROOT\"");
        if (win) sb_puts(s, " -fuse-ld=lld");
        sb_puts(s, " \"$IN\"");
        {
            int i = 0;
            for (; i < opts->nlink; i++)
                emit_llvm_link(s, opts->link_libs[i],
                               opts->link_kinds ? opts->link_kinds[i] : NULL, win);
        }
        if (!win && !wasm) sb_puts(s, " -lm");
        sb_puts(s, " -o \"$OUT\"\n");
        break;
    }
    case LLVM_OUT_JIT:
        if (opts->opt_level != LLVM_OPT_O0) {
            emit_tool(s, OPTT);
            sb_putc(s, ' ');
            sb_puts(s, O);
            sb_puts(s, " \"$IN\" -o - | ");
            emit_tool(s, LLI);
            sb_puts(s, " -\n");
        } else {
            emit_tool(s, LLI);
            sb_puts(s, " \"$IN\"\n");
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

    char in_tp[SALAM_PATH_MAX];
    to_tool_path(ll_path, in_tp, sizeof in_tp);
    bool needs_out =
        opts->output_mode != LLVM_OUT_JIT && opts->output_mode != LLVM_OUT_IR;
    char out_tp[SALAM_PATH_MAX];
    out_tp[0] = 0;
    if (needs_out && opts->output_file)
        to_tool_path(opts->output_file, out_tp, sizeof out_tp);

    sb_t script;
    sb_init(&script);
    sb_puts(&script, "set -e\n");
    sb_puts(&script, "IN=");
    sb_put_shell_arg(&script, in_tp);
    sb_putc(&script, '\n');
    if (out_tp[0]) {
        sb_puts(&script, "OUT=");
        sb_put_shell_arg(&script, out_tp);
        sb_putc(&script, '\n');
    }
    if (opts->sysroot && opts->sysroot[0]) {
        char sr_tp[SALAM_PATH_MAX];
        to_tool_path(opts->sysroot, sr_tp, sizeof sr_tp);
        sb_puts(&script, "SYSROOT=");
        sb_put_shell_arg(&script, sr_tp);
        sb_putc(&script, '\n');
    }
    emit_tool_cmd(&script, opts);

    char spath[1024];
    sal_snprintf(spath, sizeof spath, "%s.run.sh", ll_path);
    FILE *sf = fopen(spath, "wb");
    if (!sf) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), spath);
        sb_free(&script);
        return 2;
    }
    fputs(sb_cstr(&script), sf);
    fclose(sf);
    sb_free(&script);
    char script_tp[SALAM_PATH_MAX];
    to_tool_path(spath, script_tp, sizeof script_tp);
    sb_t cmd;
    sb_init(&cmd);
#if defined(_WIN32)
    sb_puts(&cmd, "wsl.exe -e bash \"");
    sb_puts(&cmd, script_tp);
    sb_puts(&cmd, "\"");
#else
    sb_puts(&cmd, "bash \"");
    sb_puts(&cmd, script_tp);
    sb_puts(&cmd, "\"");
#endif
    LOG_I(log, PH_DRIVER, "llvm toolchain: %s", sb_cstr(&cmd));
    int rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
    remove(spath);
    if (rc != 0)
        LOG_E(log, PH_DRIVER, i18n_tr("LLVM toolchain step failed (exit %d)"), rc);
    return rc;
}
