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
#include "driver/build.h"
#include "driver/llvm_build.h"
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "codegen/codegen.h"
#include "preproc/preproc.h"
#include "i18n/i18n.h"
#if defined(_WIN32)
#  include <io.h>
#else
#  include <unistd.h>
#endif

static const char *module_of(arena_t *a, const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *base = path;
    if (slash && slash + 1 > base) base = slash + 1;
    if (bslash && bslash + 1 > base) base = bslash + 1;
    const char *dot = strrchr(base, '.');
    size_t len = dot ? (size_t)(dot - base) : strlen(base);
    return arena_strndup(a, base, len);
}

static const char *dir_of(arena_t *a, const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *bs = strrchr(path, '\\');
    const char *cut = slash;
    if (bs && (!slash || bs > slash)) cut = bs;
    if (!cut) return "";
    return arena_strndup(a, path, (size_t)(cut - path));
}

static bool write_file(logger_t *log, const char *path, const char *content)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), path);
        return false;
    }
    fputs(content, f);

    fflush(f);
#if defined(_WIN32)
    _commit(_fileno(f));
#else
    fsync(fileno(f));
#endif
    fclose(f);
    return true;
}

static void emit_link(sb_t *cmd, logger_t *log, const char *spec, const char *kind,
                      bool use_tcc)
{
    bool framework = kind && strcmp(kind, "framework") == 0;
    bool is_static = kind && strcmp(kind, "static") == 0;
    if (framework) {
#if defined(__APPLE__)
        sb_puts(cmd, " -framework ");
        sb_puts(cmd, spec);
#else
        LOG_W(log, PH_DRIVER, "ignoring framework link '%s' (frameworks are macOS-only)",
              spec);
#endif
        return;
    }
    if (spec[0] == '-' || strpbrk(spec, "/\\.") != NULL) {
        sb_putc(cmd, ' ');
        sb_put_shell_arg(cmd, spec);
        return;
    }
    if (is_static && !use_tcc) {
#if defined(__APPLE__)
        LOG_W(log, PH_DRIVER,
              "static link of bare name '%s' is unsupported by the macOS linker; "
              "pass an explicit .a path. Linking it normally.",
              spec);
        sb_puts(cmd, " -l");
        sb_put_shell_arg(cmd, spec);
#else
        sb_puts(cmd, " -Wl,-Bstatic -l");
        sb_put_shell_arg(cmd, spec);
        sb_puts(cmd, " -Wl,-Bdynamic");
#endif
        return;
    }
    sb_puts(cmd, " -l");
    sb_put_shell_arg(cmd, spec);
}

int driver_build(options_t *opt)
{
    /* Cross-compilation: when a target triple is requested, hand off to the
     * LLVM backend (which honours --target) instead of the C backend + --cc. */
    if (opt->llvm_target && opt->llvm_target[0]) return driver_llvm_build(opt);

    logger_t *log = logger_new(stderr, opt->log_level, opt->color == 1);
    arena_t *arena = arena_new(1 << 20);
    int rc = 0;
    langpack_t *pack = langpack_load(opt->lang);
    if (!pack) {
        LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
        logger_free(log);
        arena_free(arena);
        return 2;
    }
    salam_set_stdlib_root(opt->stdlib_path);

    if (opt->cc && strcmp(opt->cc, "tcc") == 0) {
        static char bundled_cc[1200];
        if (salam_find_bundled_tool("gcc", bundled_cc, sizeof bundled_cc) ||
            salam_find_bundled_tool("clang", bundled_cc, sizeof bundled_cc) ||
            salam_find_bundled_tool("tcc", bundled_cc, sizeof bundled_cc)) {
            opt->cc = bundled_cc;
            LOG_I(log, PH_DRIVER, "using bundled C compiler: %s", opt->cc);
        }
    }

    const char *cfiles[SALAM_MAX_INPUTS];
    int ncfiles = 0;
    const char *generated[SALAM_MAX_INPUTS * 2 + 2];
    int ngen = 0;
    const char *first_module = NULL;
    bool all_ok = true;

    const char *entry = langpack_entry(pack);
    bool has_entry = false;

    const char *links[SALAM_MAX_INPUTS];
    const char *link_kinds[SALAM_MAX_INPUTS];
    int nlinks = 0;

    const char *defs[SALAM_MAX_INPUTS];
    int ndefs = 0;
    {
        int i = 0;
        for (; i < opt->ndefines && ndefs < SALAM_MAX_INPUTS; i++)
            defs[ndefs++] = opt->defines[i];
    }
    if (opt->asan && ndefs < SALAM_MAX_INPUTS) defs[ndefs++] = "SALAM_MEM_DEBUG";
    if (opt->asan && ndefs < SALAM_MAX_INPUTS) defs[ndefs++] = "SALAM_ASAN";

    bool use_tcc = strstr(opt->cc, "tcc") != NULL;
    if (ndefs < SALAM_MAX_INPUTS)
        defs[ndefs++] =
            use_tcc ? "SALAM_CC_TCC"
                    : (strstr(opt->cc, "clang") ? "SALAM_CC_CLANG" : "SALAM_CC_GCC");

    const char *work[SALAM_MAX_INPUTS];
    int nwork = 0;
    {
        int i = 0;
        for (; i < opt->input_count && nwork < SALAM_MAX_INPUTS; i++)
            work[nwork++] = opt->inputs[i];
    }

    {
        const char *memp = salam_resolve_import(arena, "", "mem");
        FILE *mf = memp ? fopen(memp, "rb") : NULL;
        if (mf) {
            fclose(mf);
            if (nwork < SALAM_MAX_INPUTS) work[nwork++] = memp;
        }
    }

    {
        const char *corep = salam_resolve_import(arena, "", "core");
        FILE *cf = corep ? fopen(corep, "rb") : NULL;
        if (cf) {
            fclose(cf);
            if (nwork < SALAM_MAX_INPUTS) work[nwork++] = corep;
        }
    }

    {
        const char *cryp = salam_resolve_import(arena, "", "crypto");
        FILE *yf = cryp ? fopen(cryp, "rb") : NULL;
        if (yf) {
            fclose(yf);
            if (nwork < SALAM_MAX_INPUTS) work[nwork++] = cryp;
        }
    }
    {
        int wi = 0;
        for (; wi < nwork; wi++) {
            const char *path = work[wi];
            bool dup = false;
            {
                int j = 0;
                for (; j < wi; j++)
                    if (strcmp(work[j], path) == 0) dup = true;
            }
            if (dup) continue;
            source_file_t *src = source_load(arena, path);
            if (!src) {
                LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), path);
                all_ok = false;
                continue;
            }
            src = preproc_source(arena, log, src, defs, ndefs);
            const langpack_t *modpack = langpack_detect(arena, src, pack);
            const char *modentry = langpack_entry(modpack);

            logger_set_diag_source(log, src->text, src->len, opt->diag_style,
                                   opt->diag_format);
            const char *module = module_of(arena, path);
            if (!first_module) first_module = module;
            LOG_I(log, PH_DRIVER, "compiling %s -> %s.c", path, module);
            token_stream_t *toks = NULL;
            bool lok = lexer_run(arena, log, modpack, src, &toks);
            ast_node_t *program = NULL;
            bool pok = parser_run(arena, log, toks, &program);

            {
                const char *pfiles[SALAM_MAX_INPUTS];
                int npf = salam_package_files(arena, path, pfiles, SALAM_MAX_INPUTS);
                int pi = 1;
                for (; pi < npf; pi++) {
                    source_file_t *psrc = source_load(arena, pfiles[pi]);
                    if (!psrc) {
                        LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), pfiles[pi]);
                        all_ok = false;
                        continue;
                    }
                    psrc = preproc_source(arena, log, psrc, defs, ndefs);
                    token_stream_t *ptoks = NULL;
                    if (!lexer_run(arena, log, modpack, psrc, &ptoks)) lok = false;
                    ast_node_t *pprog = NULL;
                    if (!parser_run(arena, log, ptoks, &pprog)) pok = false;
                    salam_merge_program(arena, program, pprog);
                }
            }

            sema_result_t *sr =
                sema_run(arena, log, program, src->path, langpack_code(modpack));
            if (!lok || !pok || !sr->ok) {
                all_ok = false;
                continue;
            }
            {
                size_t k = 0;
                for (; k < program->list.len; k++) {
                    ast_node_t *d = (ast_node_t *)program->list.data[k];
                    if (d->kind == AST_FUNC_DEF && d->name &&
                        strcmp(d->name, modentry) == 0) {
                        has_entry = true;
                        break;
                    }
                }
            }

            const char *idir = dir_of(arena, path);
            {
                size_t k = 0;
                for (; k < program->list.len && nwork < SALAM_MAX_INPUTS; k++) {
                    ast_node_t *d = (ast_node_t *)program->list.data[k];
                    if (d->kind == AST_LINK) {
                        const char *lib = (d->value.kind == TV_STRING && d->value.as.s)
                                              ? d->value.as.s
                                              : NULL;
                        if (lib && nlinks < SALAM_MAX_INPUTS) {
                            bool seen = false;
                            {
                                int j = 0;
                                for (; j < nlinks; j++)
                                    if (strcmp(links[j], lib) == 0) seen = true;
                            }
                            if (!seen) {
                                link_kinds[nlinks] = d->name;
                                links[nlinks++] = lib;

                                if (ndefs < SALAM_MAX_INPUTS) {
                                    size_t ll = strlen(lib);
                                    char *def = (char *)arena_alloc(arena, ll + 12);
                                    memcpy(def, "SALAM_LINK_", sizeof("SALAM_LINK_") - 1);
                                    {
                                        size_t c = 0;
                                        for (; c < ll; c++) {
                                            char ch = lib[c];
                                            if (ch >= 'a' && ch <= 'z')
                                                ch = (char)(ch - 'a' + 'A');
                                            else if (!((ch >= 'A' && ch <= 'Z') ||
                                                       (ch >= '0' && ch <= '9')))
                                                ch = '_';
                                            def[11 + c] = ch;
                                        }
                                    }
                                    def[11 + ll] = 0;
                                    bool dseen = false;
                                    {
                                        int j = 0;
                                        for (; j < ndefs; j++)
                                            if (strcmp(defs[j], def) == 0) dseen = true;
                                    }
                                    if (!dseen) defs[ndefs++] = def;
                                }
                            }
                        }
                        continue;
                    }
                    if (d->kind != AST_IMPORT) continue;
                    const char *spec = (d->value.kind == TV_STRING && d->value.as.s)
                                           ? d->value.as.s
                                           : d->name;
                    if (!spec) continue;
                    const char *ipath = salam_resolve_import(arena, idir, spec);
                    bool known = false;
                    {
                        int j = 0;
                        for (; j < nwork; j++)
                            if (strcmp(work[j], ipath) == 0) known = true;
                    }
                    if (!known) work[nwork++] = ipath;
                }
            }
            codegen_output_t *out =
                codegen_run(arena, log, program, sr, module, opt->safe, opt->debug_info,
                            src->path, modentry);
            size_t pfxlen = strlen(SALAM_MOD_PREFIX);
            size_t pathcap = pfxlen + strlen(module) + 3;
            char *cpath = (char *)arena_alloc(arena, pathcap);
            char *hpath = (char *)arena_alloc(arena, pathcap);
            sal_snprintf(cpath, pathcap, "%s%s.c", SALAM_MOD_PREFIX, module);
            sal_snprintf(hpath, pathcap, "%s%s.h", SALAM_MOD_PREFIX, module);
            if (!write_file(log, cpath, out->c_src) ||
                !write_file(log, hpath, out->h_src)) {
                all_ok = false;
                continue;
            }
            cfiles[ncfiles++] = cpath;
            generated[ngen++] = cpath;
            generated[ngen++] = hpath;
        }
    }

    if (!all_ok) {
        LOG_E(log, PH_DRIVER, i18n_tr("build aborted: errors in source"));
        if (!opt->keep_c) {
            int i = 0;
            for (; i < ngen; i++)
                remove(generated[i]);
        }
        logger_free(log);
        arena_free(arena);
        return 1;
    }

    if (opt->command != CMD_OBJ && !has_entry) {
        LOG_E(log, PH_DRIVER, i18n_tr("no entry point: define a '%s' function"), entry);
        if (!opt->keep_c) {
            int i = 0;
            for (; i < ngen; i++)
                remove(generated[i]);
        }
        logger_free(log);
        arena_free(arena);
        return 1;
    }
    int crc = 0;
    if (opt->command == CMD_OBJ) {
        const char *dbg_flag = (opt->debug_info && !strstr(opt->cc, "tcc")) ? " -g" : "";
        {
            int i = 0;
            for (; i < ncfiles && crc == 0; i++) {
                char obj[260];
                sal_snprintf(obj, sizeof(obj), "%.*s.o", (int)(strlen(cfiles[i]) - 2),
                             cfiles[i]);
                sb_t cmd;
                sb_init(&cmd);
                sb_puts(&cmd, opt->cc);
                sb_puts(&cmd, " -c -I.");
                sb_puts(&cmd, dbg_flag);
                sb_putc(&cmd, ' ');
                sb_put_shell_arg(&cmd, cfiles[i]);
                sb_puts(&cmd, " -o ");
                sb_put_shell_arg(&cmd, obj);
                LOG_I(log, PH_DRIVER, "assembling: %s", sb_cstr(&cmd));
                crc = system(sb_cstr(&cmd));
                sb_free(&cmd);
            }
        }
        if (crc != 0) {
            LOG_E(log, PH_DRIVER, i18n_tr("C compiler '%s' failed (exit %d)"), opt->cc,
                  crc);
            rc = 3;
        } else
            LOG_I(log, PH_DRIVER, "produced object files (.o)");
    } else {
        const char *output = opt->output;
        if (!output) {
            size_t ocap = strlen(first_module) + 5;
            char *o = (char *)arena_alloc(arena, ocap);
            sal_snprintf(o, ocap, "%s.exe", first_module);
            output = o;
        }
#ifdef _WIN32
        const char *lm = use_tcc ? " -lmsvcrt" : (strstr(opt->cc, "clang") ? "" : " -lm");
#else

        const char *lm = " -lm";
#endif
        sb_t cmd;
        sb_init(&cmd);
        sb_puts(&cmd, opt->cc);
        sb_puts(&cmd, " -I. -o ");
        sb_put_shell_arg(&cmd, output);
        if (opt->debug_info) {
            if (use_tcc)
                LOG_W(log, PH_DRIVER,
                      "tcc has limited debug-symbol support; prefer --cc=gcc or "
                      "--cc=clang with -g");
            else
                sb_puts(&cmd, " -g");
        }
        if (opt->asan) {
            if (use_tcc) {
                LOG_E(
                    log, PH_DRIVER,
                    "tcc does not support AddressSanitizer; use --cc=gcc or --cc=clang");
                sb_free(&cmd);
                if (!opt->keep_c) {
                    int i = 0;
                    for (; i < ngen; i++)
                        remove(generated[i]);
                }
                logger_free(log);
                arena_free(arena);
                return 2;
            }
            sb_puts(&cmd, " -fsanitize=address -fno-omit-frame-pointer");

            sb_puts(&cmd, " -DSALAM_MEM_DEBUG");
        }

        {
            int i = 0;
            for (; i < nlinks; i++) {
                sb_puts(&cmd, " -DSALAM_LINK_");
                {
                    const char *p = links[i];
                    for (; *p; p++) {
                        char c = *p;
                        if (c >= 'a' && c <= 'z')
                            c = (char)(c - 'a' + 'A');
                        else if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
                            c = '_';
                        sb_putc(&cmd, c);
                    }
                }
            }
        }
        {
            int i = 0;
            for (; i < ncfiles; i++) {
                sb_putc(&cmd, ' ');
                sb_put_shell_arg(&cmd, cfiles[i]);
            }
        }
        sb_puts(&cmd, lm);

        {
            int i = 0;
            for (; i < nlinks; i++)
                emit_link(&cmd, log, links[i], link_kinds[i], use_tcc);
        }
        LOG_I(log, PH_DRIVER, "linking: %s", sb_cstr(&cmd));
        crc = system(sb_cstr(&cmd));
        sb_free(&cmd);
        if (crc != 0) {
            LOG_E(log, PH_DRIVER, i18n_tr("C compiler '%s' failed (exit %d)"), opt->cc,
                  crc);
            rc = 3;
        } else {
            LOG_I(log, PH_DRIVER, "built executable: %s", output);

            if (opt->exe_path[0] == '\0')
                sal_snprintf(opt->exe_path, sizeof(opt->exe_path), "%s", output);
        }
    }
    if (!opt->keep_c) {
        int i = 0;
        for (; i < ngen; i++)
            remove(generated[i]);
    } else
        LOG_I(log, PH_DRIVER, "kept generated C files");
    logger_free(log);
    arena_free(arena);
    return rc;
}
