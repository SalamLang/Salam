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
#include "core/sal_format.h"
#include "core/sal_path.h"
#include "driver/llvm_build.h"
#include "driver/driver.h"
#include "driver/build.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "condcomp/condcomp.h"
#include "llvm/codegen_llvm.h"
#include "driver/llvm_toolchain.h"
#include "core/prof_self.h"
#include "i18n/i18n.h"

static const char *plural_suffix(int n)
{
    return n == 1 ? "" : "s";
}

static const char *default_output(arena_t *a, const char *module, llvm_output_mode_t m,
                                  const char *triple)
{
    /* driver_target_is_windows(), not a plain "is the triple empty" test:
       an empty triple means "no --target=", so it is this host, and reading
       it as "not Windows" would cost every native Windows build its .exe -
       bash still execs an extensionless binary, cmd.exe refuses to. */
    bool win = driver_target_is_windows(triple);
    bool msvc = triple && strstr(triple, "msvc");
    const char *ext = ".ll";
    switch (m) {
    case LLVM_OUT_ASM:
        ext = ".s";
        break;
    case LLVM_OUT_OBJ:
        ext = msvc ? ".obj" : ".o";
        break;
    case LLVM_OUT_BITCODE:
        ext = ".bc";
        break;
    case LLVM_OUT_EXEC:
        ext = win ? ".exe" : "";
        break;
    default:
        ext = ".ll";
        break;
    }
    size_t ocap = strlen(module) + strlen(ext) + 1;
    char *o = (char *)arena_alloc(a, ocap);
    sal_snprintf(o, ocap, "%s%s", module, ext);
    return o;
}

static void ll_add_link(const char **libs, const char **kinds, int *n, int cap,
                        const char *lib, const char *kind)
{
    if (!lib || *n >= cap) return;
    {
        int i = 0;
        for (; i < *n; i++)
            if (strcmp(libs[i], lib) == 0) return;
    }
    libs[*n] = lib;
    kinds[*n] = kind;
    (*n)++;
}

static void ll_scan_links(ast_node_t *prog, const char **libs, const char **kinds, int *n,
                          int cap)
{
    if (!prog) return;
    {
        size_t k = 0;
        for (; k < prog->list.len; k++) {
            ast_node_t *d = (ast_node_t *)prog->list.data[k];
            if (d->kind != AST_LINK) continue;
            const char *lib =
                (d->value.kind == TV_STRING && d->value.as.s) ? d->value.as.s : NULL;
            ll_add_link(libs, kinds, n, cap, lib, d->name);
        }
    }
}

static int ll_enqueue_imports(arena_t *a, ast_node_t *prog, const char *dir,
                              const char *lang, const char **work, int *nwork, int cap)
{
    int dropped = 0;
    if (!prog) return 0;
    {
        size_t k = 0;
        for (; k < prog->list.len; k++) {
            ast_node_t *d = (ast_node_t *)prog->list.data[k];
            if (d->kind != AST_IMPORT) continue;
            const char *ip =
                d->type_str ? d->type_str : salam_resolve_import_node(a, dir, d, lang);
            if (!ip) continue;
            bool seen = false;
            {
                int i = 0;
                for (; i < *nwork; i++)
                    if (strcmp(work[i], ip) == 0) {
                        seen = true;
                        break;
                    }
            }
            if (seen) continue;
            if (*nwork < cap)
                work[(*nwork)++] = ip;
            else
                dropped++;
        }
    }
    return dropped;
}

static int ll_gather_links(arena_t *a, logger_t *log, langpack_t *pack,
                           const char *main_path, ast_node_t *main_prog,
                           const char *main_lang, const cc_table_t *cc, const char **libs,
                           const char **kinds, int cap)
{
    int n = 0;
    const char *work[256];
    int nwork = 0, wi = 0, dropped = 0;

    ll_scan_links(main_prog, libs, kinds, &n, cap);
    dropped += ll_enqueue_imports(a, main_prog, sal_path_dir(a, main_path), main_lang,
                                  work, &nwork, 256);

    for (; wi < nwork; wi++) {
        const char *path = work[wi];
        source_file_t *src = source_load(a, path);
        if (!src) continue;
        logger_add_diag_source(log, path, src->text, src->len);
        const langpack_t *mp = langpack_detect(a, src, pack);
        token_stream_t *toks = NULL;
        lexer_run(a, log, mp, src, &toks);
        ast_node_t *prog = NULL;
        parser_run(a, log, toks, &prog);
        cc_prune_program(a, log, path, cc, prog);
        {
            const char *pf[256];
            int npf = salam_package_files(a, path, pf, 256);
            int pi = 1;
            for (; pi < npf; pi++) {
                source_file_t *ps = source_load(a, pf[pi]);
                if (!ps) continue;
                logger_add_diag_source(log, pf[pi], ps->text, ps->len);
                token_stream_t *pt = NULL;
                lexer_run(a, log, mp, ps, &pt);
                ast_node_t *pp = NULL;
                parser_run(a, log, pt, &pp);
                cc_prune_program(a, log, pf[pi], cc, pp);
                salam_merge_program(a, prog, pp);
            }
        }
        ll_scan_links(prog, libs, kinds, &n, cap);
        dropped += ll_enqueue_imports(a, prog, sal_path_dir(a, path), langpack_code(mp),
                                      work, &nwork, 256);
    }
    if (dropped > 0)
        LOG_W(log, PH_DRIVER,
              i18n_tr("import graph exceeds the %d-file scan limit; %d import%s were "
                      "skipped when collecting link directives, so some libraries may "
                      "be missing at link time"),
              (int)(sizeof work / sizeof work[0]), dropped, plural_suffix(dropped));
    return n;
}

int driver_llvm(options_t *opt)
{
    logger_t *log = logger_new(stderr, opt->log_level, resolve_color(opt->color));
    arena_t *arena = arena_new(1 << 20);
    int rc = 0;
    langpack_t *pack = langpack_load(opt->lang);
    if (!pack) {
        LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    const char *entry = langpack_entry(pack);
    salam_set_stdlib_root(opt->stdlib_path);
    if (!opt->input) {
        LOG_E(log, PH_DRIVER, i18n_tr("salam: no input file"));
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    /* Same phase names the C backend records, so --time-report/--time-trace
     * describe an LLVM build too instead of reporting an empty schedule. */
    PROF_SCOPE_BEGIN(TP_SOURCE, opt->input);
    source_file_t *src = source_load(arena, opt->input);
    PROF_SCOPE_END(TP_SOURCE);
    if (!src) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"), opt->input);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    logger_set_diag_source(log, src->text, src->len, opt->diag_style, opt->diag_format);
    logger_add_diag_source(log, opt->input, src->text, src->len);
    const char *module = sal_path_stem(arena, opt->input);
    const langpack_t *modpack = langpack_detect(arena, src, pack);
    /*
     * The entry point's name follows the language the *file* is written in,
     * not the one --lang names: an English-keyword file built with
     * --lang=fa still spells its entry `main`, not `اصلی`. The C backend
     * has always taken it from the detected pack; taking it from `pack`
     * here meant such a file got its `main` mangled like any other
     * function and the link failed with "undefined symbol: main".
     */
    entry = langpack_entry(modpack);
    token_stream_t *toks = NULL;
    PROF_SCOPE_BEGIN(TP_LEXER, opt->input);
    bool lok = lexer_run(arena, log, modpack, src, &toks);
    PROF_SCOPE_END(TP_LEXER);
    ast_node_t *program = NULL;
    PROF_SCOPE_BEGIN(TP_PARSER, opt->input);
    bool pok = parser_run(arena, log, toks, &program);
    cc_table_t *cc = cc_table_build(arena, opt->llvm_target, opt->defines, opt->ndefines);
    if (!cc_prune_program(arena, log, src->path, cc, program)) pok = false;
    PROF_SCOPE_END(TP_PARSER);
    PROF_SCOPE_BEGIN(TP_SEMANTIC, opt->input);
    sema_result_t *sr =
        sema_run(arena, log, program, src->path, langpack_code(modpack), cc);
    PROF_SCOPE_END(TP_SEMANTIC);
    if (!lok || !pok || !sr->ok) {
        LOG_E(log, PH_DRIVER, i18n_tr("build aborted: errors in source"));
        logger_free(log);
        arena_free(arena);
        return 1;
    }

    codegen_llvm_options_t o;
    codegen_llvm_options_default(&o);
    o.opt_level = (llvm_opt_level_t)opt->llvm_opt_level;
    o.output_mode = (llvm_output_mode_t)opt->llvm_emit;
    o.debug_info = opt->debug_info;
    o.verify_module = opt->llvm_verify;
    o.release = !opt->safe;
    o.target_triple = opt->llvm_target;
    o.lib_paths = opt->lib_paths;
    o.nlibpath = opt->nlibpath;
    o.native_cpu = opt->llvm_native_cpu && !(opt->llvm_target && opt->llvm_target[0]);

    const char *link_libs[SALAM_MAX_INPUTS];
    const char *link_kinds[SALAM_MAX_INPUTS];
    char sysroot_buf[1024];
    if (o.output_mode == LLVM_OUT_EXEC) {
        o.nlink =
            ll_gather_links(arena, log, pack, opt->input, program, langpack_code(modpack),
                            cc, link_libs, link_kinds, SALAM_MAX_INPUTS);
        o.link_libs = link_libs;
        o.link_kinds = link_kinds;
        if (opt->llvm_target &&
            salam_find_sysroot(opt->llvm_target, sysroot_buf, sizeof sysroot_buf)) {
            o.sysroot = sysroot_buf;
            LOG_I(log, PH_DRIVER, "using bundled sysroot: %s", o.sysroot);
        }
    }
    PROF_SCOPE_BEGIN(TP_CODEGEN, src->path);
    llvm_output_t *out =
        codegen_llvm_run_opts(arena, log, program, sr, module, entry, &o, src->path);
    PROF_SCOPE_END(TP_CODEGEN);

    bool ir_mode = (o.output_mode == LLVM_OUT_IR);
    const char *llpath;
    if (ir_mode && opt->output) {
        llpath = opt->output;
    } else {
        size_t bcap = strlen(module) + 4;
        char *buf = (char *)arena_alloc(arena, bcap);
        sal_snprintf(buf, bcap, "%s.ll", module);
        llpath = buf;
    }
    PROF_SCOPE_BEGIN(TP_WRITE, llpath);
    FILE *f = fopen(llpath, "wb");
    if (!f) {
        PROF_SCOPE_END(TP_WRITE);
        LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), llpath);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    fputs(out->ll_src, f);
    fclose(f);
    PROF_SCOPE_END(TP_WRITE);
    if (!out->ok) {
        const char *e = codegen_llvm_error(out);
        LOG_W(log, PH_DRIVER,
              "%s; the program uses constructs the LLVM backend does not yet "
              "support, so %s may be incomplete",
              e ? e : "unsupported construct", llpath);
        /*
         * The partial .ll is written above so it can be inspected, but for a
         * build that is about to fall back to the C backend it is just
         * litter in the user's working directory - keep it only when the
         * user asked for intermediates, or when it *is* the requested
         * output (`salam llvm -o foo.ll`).
         */
        if (!opt->keep_c && !(ir_mode && opt->output)) remove(llpath);
        logger_free(log);
        arena_free(arena);
        return SALAM_RC_LLVM_UNSUPPORTED;
    }
    if (ir_mode) {
        if (o.verify_module || o.opt_level != LLVM_OPT_O0)
            rc = salam_llvm_toolchain(log, llpath, &o);
        if (rc == 0) {
            LOG_I(log, PH_DRIVER, "LLVM IR written to %s", llpath);
            LOG_I(log, PH_DRIVER,
                  "  native: clang %s -lm -o %s | wasm: clang --target=wasm32-wasi ...",
                  llpath, module);
        }
    } else if (o.output_mode == LLVM_OUT_JIT) {
        rc = salam_llvm_toolchain(log, llpath, &o);
        if (!opt->keep_c) remove(llpath);
    } else {
        o.output_file = opt->output
                            ? opt->output
                            : default_output(arena, driver_output_stem(arena, opt->input),
                                             o.output_mode, opt->llvm_target);
        /* A caller-named executable still has to be launchable on the
           target: `--target=x86_64-w64-windows-gnu -o hello` writes
           hello.exe from a Linux host, while a non-Windows target keeps the
           name exactly as typed. Executables only - an -o for .o/.ll/.s
           names that artifact and nothing is appended to it. */
        if (opt->output && o.output_mode == LLVM_OUT_EXEC)
            o.output_file = driver_exe_name(arena, o.output_file, opt->llvm_target);
        rc = salam_llvm_toolchain(log, llpath, &o);
        if (!opt->keep_c) remove(llpath);
        if (rc == 0) {
            LOG_I(log, PH_DRIVER, i18n_tr("wrote %s"), o.output_file);
            if (opt->exe_path[0] == '\0' && o.output_mode == LLVM_OUT_EXEC)
                sal_snprintf(opt->exe_path, sizeof(opt->exe_path), "%s", o.output_file);
        } else if (o.output_mode == LLVM_OUT_EXEC &&
                   !(opt->llvm_target && opt->llvm_target[0])) {
            /* Native link only: a cross-compile has no C backend to retry
             * with, so it keeps the plain failure code. */
            rc = SALAM_RC_LLVM_LINK_FAILED;
        }
    }
    logger_free(log);
    arena_free(arena);
    return rc;
}

int driver_llvm_build(options_t *opt)
{
    opt->llvm_emit = (opt->command == CMD_OBJ) ? (int)LLVM_OUT_OBJ : (int)LLVM_OUT_EXEC;
    int rc = driver_llvm(opt);
    /*
     * Only a genuine cross-compile failure gets the toolchain hint. A native
     * build never asked for a target, and SALAM_RC_LLVM_UNSUPPORTED is
     * handled by driver_build() (which falls back to the C backend), so
     * neither should be reported as a missing cross toolchain.
     */
    if (rc != 0 && rc != SALAM_RC_LLVM_UNSUPPORTED && rc != SALAM_RC_LLVM_LINK_FAILED &&
        opt->llvm_target && opt->llvm_target[0]) {
        fprintf(stderr,
                i18n_tr("salam: cross-compilation for target '%s' failed; see the "
                        "diagnostics above. If they point to a missing toolchain, "
                        "install the LLVM tools (clang/llc), plus lld and a sysroot for "
                        "Windows targets.\n"),
                opt->llvm_target);
    }
    return rc;
}
