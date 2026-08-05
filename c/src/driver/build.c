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
#include "driver/build.h"
#include "driver/driver.h"
#include "driver/llvm_build.h"
#include "core/arena.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "semantic/dce.h"
#include "codegen/codegen.h"
#include "condcomp/condcomp.h"
#include "i18n/i18n.h"

#if defined(_WIN32)
#  include <io.h>
#  include <sys/stat.h>
#else
#  include <unistd.h>
#  include <sys/stat.h>
#  include <dirent.h>
#endif

#ifdef SALAM_HAVE_EMBED_HOSTLIBS
#  include "driver/embed_sysroot.h"
extern const unsigned char salam_embed_hostlibs[];
extern const unsigned char salam_embed_hostlibs_end[];
#endif

/*
 * Extra `-L` search directory for the embedded static third-party libs
 * built for this host (SALAM_EMBED_HOSTLIBS_DIR in c/Makefile) - this
 * whole file only ever runs for a plain native build (driver_build()
 * redirects to driver_llvm_build() before reaching here whenever
 * --target= is set), so no target-triple check is needed, unlike the
 * equivalent salam_try_embed_hostlibs() in llvm_native.c. Returns 0 (out
 * left untouched) when nothing was embedded - optional, not an error.
 */
static int try_embed_hostlibs(logger_t *log, char *out, size_t outn)
{
    (void)log;
    (void)out;
    (void)outn;
#ifdef SALAM_HAVE_EMBED_HOSTLIBS
    if (salam_materialize_sysroot(
            "hostlibs", salam_embed_hostlibs,
            (size_t)(salam_embed_hostlibs_end - salam_embed_hostlibs), out, outn)) {
        LOG_I(log, PH_DRIVER, "using embedded static third-party libs: %s", out);
        return 1;
    }
#endif
    return 0;
}

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

static bool copy_file_bin(const char *src, const char *dst)
{
    FILE *in = fopen(src, "rb");
    if (!in) return false;
    FILE *out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return false;
    }
    char buf[65536];
    size_t n;
    bool ok = true;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            ok = false;
            break;
        }
    }
    if (ferror(in)) ok = false;
    fclose(in);
    fclose(out);
    return ok;
}

/* try_embed_hostlibs() only gets the static archive a linker needs at build
 * time onto its `-L` path - a dynamically-linked host lib (sqlite3.dll on
 * this Windows/tcc combination, since tcc cannot read GCC's static .a/
 * .dll.a archives at all - "invalid object file" / "cannot find" - but
 * CAN link directly against a real DLL by reading its export table, the
 * same trick that makes `-lwinhttp`/`-lws2_32` work against
 * C:\Windows\System32 with no import lib in sight) still needs to be
 * next to the *produced executable* at run time, or Windows' loader
 * fails with "cannot open shared object file" the moment the program
 * calls into it - none of hostlibs_dir/System32/PATH is guaranteed to be
 * on the loader's search path for wherever `output` ends up. Copying any
 * shared libs found in hostlibs_dir alongside `output` after a
 * successful link closes that gap; a no-op when hostlibs weren't used
 * (hostlibs_dir NULL) or contain no shared libs (e.g. a Unix build,
 * where the embedded lib is a plain .a and this step is unnecessary).
 */
static void copy_hostlib_shared_libs(logger_t *log, arena_t *a, const char *hostlibs_dir,
                                     const char *output)
{
    if (!hostlibs_dir || !hostlibs_dir[0]) return;
    const char *dest_dir = dir_of(a, output);
    const char *out_name = strrchr(output, '/');
    {
        const char *bs = strrchr(output, '\\');
        if (bs && (!out_name || bs > out_name)) out_name = bs;
    }
    out_name = out_name ? out_name + 1 : output;
#if defined(_WIN32)
    char pattern[1024];
    sal_snprintf(pattern, sizeof pattern, "%s\\*.dll", hostlibs_dir);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pattern, &fd);
    if (h == -1) return;
    do {
        if (fd.attrib & _A_SUBDIR) continue;
        char src[1200], dst[1200];
        sal_snprintf(src, sizeof src, "%s/%s", hostlibs_dir, fd.name);
        if (dest_dir[0])
            sal_snprintf(dst, sizeof dst, "%s/%s", dest_dir, fd.name);
        else
            sal_snprintf(dst, sizeof dst, "%s", fd.name);
        if (strcmp(dst, output) == 0 || strcmp(fd.name, out_name) == 0)
            continue; /* never overwrite the binary we just linked */
        if (!copy_file_bin(src, dst))
            LOG_W(log, PH_DRIVER, "could not copy '%s' next to '%s'", src, output);
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(hostlibs_dir);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL) {
        size_t L = strlen(e->d_name);
        bool is_so = L > 3 && strstr(e->d_name, ".so") != NULL;
        if (!is_so) continue;
        char src[1200], dst[1200];
        sal_snprintf(src, sizeof src, "%s/%s", hostlibs_dir, e->d_name);
        if (dest_dir[0])
            sal_snprintf(dst, sizeof dst, "%s/%s", dest_dir, e->d_name);
        else
            sal_snprintf(dst, sizeof dst, "%s", e->d_name);
        if (strcmp(dst, output) == 0 || strcmp(e->d_name, out_name) == 0) continue;
        if (!copy_file_bin(src, dst))
            LOG_W(log, PH_DRIVER, "could not copy '%s' next to '%s'", src, output);
    }
    closedir(d);
#endif
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

static bool link_spec_is_path(const char *spec)
{
    if (strpbrk(spec, "/\\")) return true;
    static const char *const exts[] = {".a", ".so", ".dll", ".lib", ".dylib"};
    size_t n = strlen(spec);
    size_t i = 0;
    for (; i < sizeof(exts) / sizeof(exts[0]); i++) {
        size_t el = strlen(exts[i]);
        if (n > el && !strcmp(spec + n - el, exts[i])) return true;
    }
    return false;
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
    if (spec[0] == '-' || link_spec_is_path(spec)) {
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

#if !defined(_WIN32)
typedef struct {
    bool active;
    char tcc_dir[1024];
    char musl_dir[1024];
} bundled_musl_tcc_t;

static bundled_musl_tcc_t detect_bundled_musl_tcc(const char *cc_path)
{
    bundled_musl_tcc_t r;
    r.active = false;
    r.tcc_dir[0] = '\0';
    r.musl_dir[0] = '\0';
    if (!strstr(cc_path, "tcc")) return r;
    const char *slash = strrchr(cc_path, '/');
    if (!slash) return r;
    sal_snprintf(r.tcc_dir, sizeof r.tcc_dir, "%.*s", (int)(slash - cc_path), cc_path);
    char probe[1200];
    sal_snprintf(probe, sizeof probe, "%s/musl/lib/crt1.o", r.tcc_dir);
    FILE *f = fopen(probe, "rb");
    if (!f) return r;
    fclose(f);
    sal_snprintf(r.musl_dir, sizeof r.musl_dir, "%s/musl", r.tcc_dir);
    r.active = true;
    return r;
}
#endif

static const char *plural_suffix(int n)
{
    return n == 1 ? "" : "s";
}

static bool path_is_dir(const char *p)
{
#if defined(_WIN32)
    struct _stat st;
    return _stat(p, &st) == 0 && (st.st_mode & _S_IFDIR) != 0;
#else
    struct stat st;
    return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

/* driver.c's list_salam_files, generalized to an arbitrary directory (still
 * bare filenames when dir=="." for backward compat with that cwd-only
 * caller; dir-prefixed otherwise) - no recursion either way. */
static void list_salam_files_in(arena_t *a, const char *dir, const char **out, int *n)
{
    *n = 0;
    bool is_cwd = strcmp(dir, ".") == 0;
    /* Strip a trailing slash (`salam build ./compiler/` passes one) so the
     * joined path below doesn't come out as "./compiler//main.salam" - a
     * string that dedup-by-equality import resolution elsewhere doesn't
     * recognize as the same file as the canonical one, causing the entry
     * file to be compiled twice under two different-looking paths. */
    char trimmed[1024];
    sal_snprintf(trimmed, sizeof trimmed, "%s", dir);
    size_t tlen = strlen(trimmed);
    if (tlen > 0 && (trimmed[tlen - 1] == '/' || trimmed[tlen - 1] == '\\'))
        trimmed[tlen - 1] = '\0';
#if defined(_WIN32)
    char pattern[1024];
    sal_snprintf(pattern, sizeof pattern, "%s\\*.salam", trimmed);
    struct _finddata_t fd;
    intptr_t h = _findfirst(is_cwd ? "*.salam" : pattern, &fd);
    if (h == -1) return;
    do {
        if (!(fd.attrib & _A_SUBDIR) && *n < SALAM_MAX_INPUTS) {
            if (is_cwd)
                out[(*n)++] = arena_strdup(a, fd.name);
            else {
                char full[1024];
                sal_snprintf(full, sizeof full, "%s/%s", trimmed, fd.name);
                out[(*n)++] = arena_strdup(a, full);
            }
        }
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(trimmed);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && *n < SALAM_MAX_INPUTS) {
        size_t L = strlen(e->d_name);
        if (L > 6 && strcmp(e->d_name + L - 6, ".salam") == 0) {
            if (is_cwd)
                out[(*n)++] = arena_strdup(a, e->d_name);
            else {
                char full[1024];
                sal_snprintf(full, sizeof full, "%s/%s", trimmed, e->d_name);
                out[(*n)++] = arena_strdup(a, full);
            }
        }
    }
    closedir(d);
#endif
}

static bool file_has_entry(arena_t *a, langpack_t *pack, const char *entry,
                           const char *path)
{
    logger_t *quiet = logger_new(stderr, LOG_OFF, false);
    source_file_t *src = source_load(a, path);
    bool found = false;
    if (src) {
        token_stream_t *toks = NULL;
        lexer_run(a, quiet, pack, src, &toks);
        ast_node_t *program = NULL;
        parser_run(a, quiet, toks, &program);
        if (program) {
            cc_table_t *cc = cc_table_build(a, NULL, NULL, 0);
            cc_prune_program(a, quiet, path, cc, program);
            size_t i = 0;
            for (; i < program->list.len; i++) {
                ast_node_t *d = (ast_node_t *)program->list.data[i];
                if (d->kind == AST_FUNC_DEF && d->name && strcmp(d->name, entry) == 0) {
                    found = true;
                    break;
                }
            }
        }
    }
    logger_free(quiet);
    return found;
}

/* Shared "scan dir for exactly one file defining `entry`" used by driver_build
 * when no input was given (dir=".") or an explicit directory was passed
 * instead of a file. Logs its own error and returns NULL on failure (0 or >1
 * matches); returns the resolved path (arena-owned) on success. */
static const char *resolve_dir_entry(arena_t *arena, logger_t *log, langpack_t *pack,
                                     const char *dir)
{
    const char *entry = langpack_entry(pack);
    const char *files[SALAM_MAX_INPUTS];
    int nfiles = 0;
    list_salam_files_in(arena, dir, files, &nfiles);
    if (nfiles == 0) {
        LOG_E(log, PH_DRIVER, i18n_tr("no .salam files found in the current directory"));
        return NULL;
    }
    const char *entries[SALAM_MAX_INPUTS];
    int nentries = 0;
    {
        int i = 0;
        for (; i < nfiles; i++)
            if (file_has_entry(arena, pack, entry, files[i]))
                entries[nentries++] = files[i];
    }
    if (nentries == 0) {
        LOG_E(log, PH_DRIVER,
              i18n_tr("no entry point: none of the %d .salam file%s here defines a "
                      "'%s' function"),
              nfiles, plural_suffix(nfiles), entry);
        return NULL;
    }
    if (nentries > 1) {
        LOG_E(log, PH_DRIVER, i18n_tr("ambiguous entry point: %d files define '%s':"),
              nentries, entry);
        int i = 0;
        for (; i < nentries; i++)
            fprintf(stderr, "    %s\n", entries[i]);
        fprintf(stderr, "  run a specific one with: salam run <file.salam>\n");
        return NULL;
    }
    LOG_I(log, PH_DRIVER, i18n_tr("entry point: %s"), entries[0]);
    return entries[0];
}

int driver_build(options_t *opt)
{
    if (opt->llvm_target && opt->llvm_target[0]) return driver_llvm_build(opt);

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
    /* A bare `salam build` (no inputs) or a directory passed as the sole
     * input (`salam build .`, `salam build ./compiler/`) doesn't name a
     * file - scan that directory's top-level .salam files for the one that
     * defines `main` and build that, same auto-detection driver_run()
     * already does for a bare `salam run`. Without this, a directory input
     * used to fall through to the per-file work-loop below, which reads it
     * as a (nonexistent) source file instead of "build the program in this
     * directory". */
    if (opt->input_count == 0) {
        const char *first = resolve_dir_entry(arena, log, pack, ".");
        if (!first) {
            logger_free(log);
            arena_free(arena);
            return 2;
        }
        opt->inputs[0] = first;
        opt->input_count = 1;
    } else if (opt->input_count == 1 && path_is_dir(opt->inputs[0])) {
        const char *first = resolve_dir_entry(arena, log, pack, opt->inputs[0]);
        if (!first) {
            logger_free(log);
            arena_free(arena);
            return 2;
        }
        opt->inputs[0] = first;
    }

    salam_set_stdlib_root(opt->stdlib_path);

#if !defined(_WIN32)
    bundled_musl_tcc_t musl_tcc;
    musl_tcc.active = false;
#endif
    if (opt->cc && strcmp(opt->cc, "tcc") == 0) {
        static char bundled_cc[1200];
        if (salam_find_bundled_tool("gcc", bundled_cc, sizeof bundled_cc) ||
            salam_find_bundled_tool("clang", bundled_cc, sizeof bundled_cc) ||
            salam_find_bundled_tool("tcc", bundled_cc, sizeof bundled_cc)) {
            opt->cc = bundled_cc;
            LOG_I(log, PH_DRIVER, "using bundled C compiler: %s", opt->cc);
#if !defined(_WIN32)
            musl_tcc = detect_bundled_musl_tcc(bundled_cc);
            if (musl_tcc.active)
                LOG_I(log, PH_DRIVER, "using bundled musl sysroot: %s",
                      musl_tcc.musl_dir);
#endif
        }
    }

    const char *scratch = salam_scratch_dir();
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

    ast_node_t *b_programs[SALAM_MAX_INPUTS] = {0};
    sema_result_t *b_sr[SALAM_MAX_INPUTS] = {0};
    const char *b_module[SALAM_MAX_INPUTS] = {0};
    const char *b_modentry[SALAM_MAX_INPUTS] = {0};
    const char *b_pkg[SALAM_MAX_INPUTS] = {0};
    const char *b_srcpath[SALAM_MAX_INPUTS] = {0};
    int nb = 0;

    dce_reset(arena);
    dce_enable();

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
                LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"), path);
                all_ok = false;
                continue;
            }
            cc_table_t *cc = cc_table_build(arena, NULL, defs, ndefs);
            const langpack_t *modpack = langpack_detect(arena, src, pack);
            const char *modentry = langpack_entry(modpack);

            logger_set_diag_source(log, src->text, src->len, opt->diag_style,
                                   opt->diag_format);
            logger_add_diag_source(log, path, src->text, src->len);
            const char *module = module_of(arena, path);
            if (!first_module) first_module = module;
            LOG_I(log, PH_DRIVER, "compiling %s -> %s.c", path, module);
            token_stream_t *toks = NULL;
            bool lok = lexer_run(arena, log, modpack, src, &toks);
            ast_node_t *program = NULL;
            bool pok = parser_run(arena, log, toks, &program);
            if (!cc_prune_program(arena, log, path, cc, program)) pok = false;

            {
                const char *pfiles[SALAM_MAX_INPUTS];
                int npf = salam_package_files(arena, path, pfiles, SALAM_MAX_INPUTS);
                int pi = 1;
                for (; pi < npf; pi++) {
                    source_file_t *psrc = source_load(arena, pfiles[pi]);
                    if (!psrc) {
                        LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"),
                              pfiles[pi]);
                        all_ok = false;
                        continue;
                    }
                    logger_add_diag_source(log, pfiles[pi], psrc->text, psrc->len);
                    token_stream_t *ptoks = NULL;
                    if (!lexer_run(arena, log, modpack, psrc, &ptoks)) lok = false;
                    ast_node_t *pprog = NULL;
                    if (!parser_run(arena, log, ptoks, &pprog)) pok = false;
                    if (!cc_prune_program(arena, log, pfiles[pi], cc, pprog)) pok = false;
                    salam_merge_program(arena, program, pprog);
                }
            }

            sema_result_t *sr =
                sema_run(arena, log, program, src->path, langpack_code(modpack), cc);
            if (!lok || !pok || !sr->ok) {
                all_ok = false;
                continue;
            }
            const char *modpkg = program->name ? program->name : "main";
            {
                size_t k = 0;
                for (; k < program->list.len; k++) {
                    ast_node_t *d = (ast_node_t *)program->list.data[k];
                    if (d->kind == AST_FUNC_DEF && d->name &&
                        strcmp(d->name, modentry) == 0) {
                        has_entry = true;
                        dce_mark_root(modpkg, modentry);
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
                    const char *ipath =
                        d->type_str ? d->type_str
                                    : salam_resolve_import_node(arena, idir, d,
                                                                langpack_code(modpack));
                    if (!ipath) continue;
                    bool known = false;
                    {
                        int j = 0;
                        for (; j < nwork; j++)
                            if (strcmp(work[j], ipath) == 0) known = true;
                    }
                    if (!known) work[nwork++] = ipath;
                }
            }
            b_programs[wi] = program;
            b_sr[wi] = sr;
            b_module[wi] = module;
            b_modentry[wi] = modentry;
            b_pkg[wi] = modpkg;
            b_srcpath[wi] = src->path;
            if (wi + 1 > nb) nb = wi + 1;
        }
    }

    dce_finish();

    {
        int wi = 0;
        for (; wi < nb; wi++) {
            if (!b_programs[wi]) continue;
            ast_node_t *program = b_programs[wi];
            sema_result_t *sr = b_sr[wi];
            const char *module = b_module[wi];
            const char *modentry = b_modentry[wi];
            const char *modpkg = b_pkg[wi];

            {
                vec_t kept;
                vec_init(&kept);
                size_t k = 0;
                for (; k < program->list.len; k++) {
                    ast_node_t *d = (ast_node_t *)program->list.data[k];
                    bool keep = true;
                    if (d->kind == AST_FUNC_DEF && !d->is_extern && !d->synthetic &&
                        d->typarams.len == 0 &&
                        !(d->name && strcmp(d->name, modentry) == 0))
                        keep = dce_reachable(modpkg, d->name);
                    if (keep) vec_push(arena, &kept, d);
                }
                program->list = kept;
            }

            codegen_output_t *out =
                codegen_run(arena, log, program, sr, module, opt->safe, opt->debug_info,
                            b_srcpath[wi], modentry, opt->llvm_target);
            size_t pfxlen = strlen(SALAM_MOD_PREFIX);
            size_t pathcap = strlen(scratch) + 1 + pfxlen + strlen(module) + 3;
            char *cpath = (char *)arena_alloc(arena, pathcap);
            char *hpath = (char *)arena_alloc(arena, pathcap);
            sal_snprintf(cpath, pathcap, "%s/%s%s.c", scratch, SALAM_MOD_PREFIX, module);
            sal_snprintf(hpath, pathcap, "%s/%s%s.h", scratch, SALAM_MOD_PREFIX, module);
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
    const char *opt_flag = (!opt->debug_info && !opt->asan && !strstr(opt->cc, "tcc") &&
                            !strstr(opt->cc, "-O"))
                               ? " -O2"
                               : "";
    bool optimizing_build = !opt->debug_info && !opt->asan && !strstr(opt->cc, "tcc") &&
                            !strstr(opt->cc, "-O0");
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
#if !defined(_WIN32)
                if (musl_tcc.active) {
                    sb_puts(&cmd, " -B");
                    sb_put_shell_arg(&cmd, musl_tcc.tcc_dir);
                    sb_puts(&cmd, " -I");
                    sb_put_shell_arg(&cmd, musl_tcc.tcc_dir);
                    sb_puts(&cmd, "/include -I");
                    sb_put_shell_arg(&cmd, musl_tcc.musl_dir);
                    sb_puts(&cmd, "/include");
                }
#endif
                sb_puts(&cmd, " -c -I. -I");
                sb_put_shell_arg(&cmd, scratch);
                sb_puts(&cmd, opt_flag);
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
        const char *lto_flag = "";
        if (optimizing_build && !use_tcc) {
#ifdef _WIN32
            if (!strstr(opt->cc, "clang")) lto_flag = " -flto";
#else
            lto_flag = " -flto";
#endif
        }
        char hostlibs[1024];
        hostlibs[0] = '\0';
        sb_t cmd;
        sb_init(&cmd);
        sb_puts(&cmd, opt->cc);
#if !defined(_WIN32)
        if (musl_tcc.active) {
            sb_puts(&cmd, " -B");
            sb_put_shell_arg(&cmd, musl_tcc.tcc_dir);
            sb_puts(&cmd, " -nostdlib -static -I");
            sb_put_shell_arg(&cmd, musl_tcc.tcc_dir);
            sb_puts(&cmd, "/include -I");
            sb_put_shell_arg(&cmd, musl_tcc.musl_dir);
            sb_puts(&cmd, "/include -L");
            sb_put_shell_arg(&cmd, musl_tcc.musl_dir);
            sb_puts(&cmd, "/lib");
        }
#endif
        sb_puts(&cmd, opt_flag);
        sb_puts(&cmd, lto_flag);
        sb_puts(&cmd, " -I. -I");
        sb_put_shell_arg(&cmd, scratch);
        sb_puts(&cmd, " -o ");
        sb_put_shell_arg(&cmd, output);
#if defined(_WIN32)
        /* tcc's default linked stack (SizeOfStackReserve in the PE header)
         * is only 1MiB on this target - half of MinGW gcc's 2MiB default -
         * which is too little for the self-hosted compiler's own recursive
         * descent parser/semantic pass to process its own (large, deeply
         * nested) source: a tcc-linked salam.exe building the self-hosted
         * compiler/ sources with --cc=tcc reliably stack-overflows partway
         * through parsing,
         * while the identical workload run from a gcc-linked salam.exe
         * (2MiB stack) does not. Confirmed via objdump -p on minimal tcc
         * output (SizeOfStackReserve 0x100000) vs the same source built by
         * gcc (0x200000), and via a direct A/B: a tcc-linked self-hosted
         * binary crashes rebuilding itself; the same binary relinked with
         * gcc instead does not. -Wl,--stack tells tcc's linker to reserve
         * more (8MiB, matching common *nix default thread stack sizes)
         * regardless of which --cc built the binary doing the *compiling*.
         */
        if (use_tcc) sb_puts(&cmd, " -Wl,--stack=8388608");
        /* codegen_header.c per-module prelude marks salam_ob, salam_obn,
         * salam_out_flush and salam_out_fini as weak, so every module can
         * carry its own copy and the linker picks one - standard
         * practice, reliable on ELF. On this MinGW/GCC plus binutils
         * combination, weak data symbols (salam_ob/salam_obn) sometimes
         * still collide at link time with a multiple-definition error;
         * reproduced building tests general web_router.salam (query.o
         * vs url.o). tcc takes the plain non-weak fallback in that same
         * prelude, so it never hits this; the allow-multiple-definition
         * linker flag restores the intended any-one-definition-is-fine
         * behavior for gcc/clang on Windows. */
        if (!use_tcc) sb_puts(&cmd, " -Wl,--allow-multiple-definition");
#endif
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
#if !defined(_WIN32)
        if (musl_tcc.active) {
            char crtobj[1200];
            sal_snprintf(crtobj, sizeof crtobj, "%s/lib/crt1.o", musl_tcc.musl_dir);
            sb_putc(&cmd, ' ');
            sb_put_shell_arg(&cmd, crtobj);
            sal_snprintf(crtobj, sizeof crtobj, "%s/lib/crti.o", musl_tcc.musl_dir);
            sb_putc(&cmd, ' ');
            sb_put_shell_arg(&cmd, crtobj);
        }
#endif
        {
            int i = 0;
            for (; i < ncfiles; i++) {
                sb_putc(&cmd, ' ');
                sb_put_shell_arg(&cmd, cfiles[i]);
            }
        }
        sb_puts(&cmd, lm);

        if (try_embed_hostlibs(log, hostlibs, sizeof hostlibs)) {
            sb_puts(&cmd, " -L");
            sb_put_shell_arg(&cmd, hostlibs);
        }

        {
            int i = 0;
            for (; i < nlinks; i++)
                emit_link(&cmd, log, links[i], link_kinds[i], use_tcc);
        }
#if !defined(_WIN32)
        if (musl_tcc.active) {
            char libpath[1200];
            sal_snprintf(libpath, sizeof libpath, "%s/lib/libgcc.a", musl_tcc.musl_dir);
            sb_puts(&cmd, " -lc ");
            sb_put_shell_arg(&cmd, libpath);
            sb_putc(&cmd, ' ');
            sal_snprintf(libpath, sizeof libpath, "%s/libtcc1.a", musl_tcc.tcc_dir);
            sb_put_shell_arg(&cmd, libpath);
            sal_snprintf(libpath, sizeof libpath, "%s/lib/libgcc.a", musl_tcc.musl_dir);
            sb_puts(&cmd, " -lc ");
            sb_put_shell_arg(&cmd, libpath);
            sal_snprintf(libpath, sizeof libpath, "%s/lib/crtn.o", musl_tcc.musl_dir);
            sb_putc(&cmd, ' ');
            sb_put_shell_arg(&cmd, libpath);
        }
#endif
        LOG_I(log, PH_DRIVER, "linking: %s", sb_cstr(&cmd));
        crc = system(sb_cstr(&cmd));
        sb_free(&cmd);
        if (crc != 0) {
            LOG_E(log, PH_DRIVER, i18n_tr("C compiler '%s' failed (exit %d)"), opt->cc,
                  crc);
            rc = 3;
        } else {
            LOG_I(log, PH_DRIVER, "built executable: %s", output);
            copy_hostlib_shared_libs(log, arena, hostlibs, output);

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
