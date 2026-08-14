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
#include "driver/build.h"
#include "driver/driver.h"
#include "driver/llvm_build.h"
#include "llvm/llvm_native.h"
#include "core/arena.h"
#include "core/prof_self.h"
#include "core/sb.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "semantic/dce.h"
#include "ast/ast.h"
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

/* True when `filename` (a hostlibs directory entry, e.g. "sqlite3.dll")
 * looks like it belongs to one of the `-l` names this specific build
 * requested (e.g. "sqlite3"). Deliberately conservative - substring, not
 * exact-basename, matching (so "libsqlite3-0.dll" still matches a
 * request for "sqlite3") - since this only gates an extra file copy, not
 * anything correctness-critical; a false positive costs a few KB of
 * needless copying, a false negative costs a "DLL not found" at run
 * time, so this errs toward matching. */
static bool hostlib_file_wanted(const char *filename, const char **links, int nlinks)
{
    int i = 0;
    for (; i < nlinks; i++) {
        if (strstr(filename, links[i])) return true;
    }
    return false;
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
 * on the loader's search path for wherever `output` ends up. Copying the
 * shared libs THIS BUILD ACTUALLY LINKED (via `links`/`nlinks` - not a
 * blanket copy of everything in hostlibs_dir) alongside `output` after a
 * successful link closes that gap without taxing every other build in
 * the embedded hostlibs directory's file-copy cost; a no-op when
 * hostlibs weren't used (hostlibs_dir NULL), nothing linked matches one
 * (a Unix build, say, where the embedded lib is a plain .a and this step
 * is unnecessary - or simply a build that never asked for sqlite3).
 */
static void copy_hostlib_shared_libs(logger_t *log, arena_t *a, const char *hostlibs_dir,
                                     const char *output, const char **links, int nlinks)
{
    if (!hostlibs_dir || !hostlibs_dir[0] || nlinks <= 0) return;
    const char *dest_dir = sal_path_dir(a, output);
    const char *out_name = sal_path_base(output);
#if defined(_WIN32)
    char pattern[1100];
    sal_path_join(pattern, sizeof pattern, hostlibs_dir, "*.dll");
    struct _finddata_t fd;
    intptr_t h = _findfirst(pattern, &fd);
    if (h == -1) return;
    do {
        if (fd.attrib & _A_SUBDIR) continue;
        if (!hostlib_file_wanted(fd.name, links, nlinks)) continue;
        char src[1200], dst[1200];
        sal_path_join(src, sizeof src, hostlibs_dir, fd.name);
        sal_path_join(dst, sizeof dst, dest_dir, fd.name);
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
        if (!hostlib_file_wanted(e->d_name, links, nlinks)) continue;
        char src[1200], dst[1200];
        sal_path_join(src, sizeof src, hostlibs_dir, e->d_name);
        sal_path_join(dst, sizeof dst, dest_dir, e->d_name);
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

/* True when 'path' already holds exactly 'content'. Used to leave an unchanged
   generated file (and therefore its .o) untouched on an incremental build. */
static bool file_has_content(const char *path, const char *content)
{
    FILE *f = fopen(path, "rb");
    size_t want = strlen(content);
    size_t off = 0;
    char buf[4096];
    if (!f) return false;
    while (off < want) {
        size_t chunk = want - off;
        size_t got;
        if (chunk > sizeof(buf)) chunk = sizeof(buf);
        got = fread(buf, 1, chunk, f);
        if (got == 0 || memcmp(buf, content + off, got) != 0) {
            fclose(f);
            return false;
        }
        off += got;
    }
    /* Must be EOF: a file that starts with 'content' but is longer differs. */
    {
        bool eof = fread(buf, 1, 1, f) == 0;
        fclose(f);
        return eof;
    }
}

static bool file_exists(const char *path)
{
    FILE *f = fopen(path, "rb");
    if (!f) return false;
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
    if (sal_path_dir_buf(cc_path, r.tcc_dir, sizeof r.tcc_dir) == 0) return r;
    char probe[1200];
    sal_path_join(probe, sizeof probe, r.tcc_dir, "musl/lib/crt1.o");
    FILE *f = fopen(probe, "rb");
    if (!f) return r;
    fclose(f);
    sal_path_join(r.musl_dir, sizeof r.musl_dir, r.tcc_dir, "musl");
    r.active = true;
    return r;
}
#endif

static const char *plural_suffix(int n)
{
    return n == 1 ? "" : "s";
}

bool driver_path_is_dir(const char *p)
{
#if defined(_WIN32)
    struct _stat st;
    return _stat(p, &st) == 0 && (st.st_mode & _S_IFDIR) != 0;
#else
    struct stat st;
    return stat(p, &st) == 0 && S_ISDIR(st.st_mode);
#endif
}

static bool path_is_file(const char *p)
{
#if defined(_WIN32)
    struct _stat st;
    return _stat(p, &st) == 0 && (st.st_mode & _S_IFDIR) == 0;
#else
    struct stat st;
    return stat(p, &st) == 0 && S_ISREG(st.st_mode);
#endif
}

const char *driver_project_entry_file(arena_t *a, const char *dir)
{
    /* join trims a trailing separator (`salam build ./compiler/`) and
       normalize drops a leading "./", so the entry file comes out spelled
       exactly one way whatever the caller typed - the dedup-by-equality
       import resolution downstream depends on that. */
    char buf[1200];
    sal_path_join(buf, sizeof buf, dir, SALAM_PROJECT_FILE);
    sal_path_normalize(buf);
    if (!path_is_file(buf)) return NULL;
    return arena_strdup(a, buf);
}

const char *driver_output_stem(arena_t *a, const char *path)
{
    const char *module = sal_path_stem(a, path);
    if (strcmp(module, "salam") != 0) return module;
    const char *dir = sal_path_dir(a, path);
    if (!dir[0]) dir = ".";
    /* Resolve to an absolute path on the heap - a fixed local buffer is
     * both unsafe (realpath() writes up to PATH_MAX and cannot be told the
     * caller's size) and too small (Windows long paths reach ~32k), so
     * realpath() allocates the exact answer and _fullpath(), which only
     * fails with ERANGE when the result does not fit, is retried on a
     * growing buffer. */
#if defined(_WIN32)
    char *abs = NULL;
    {
        size_t cap = 1024;
        for (; cap <= 65536; cap *= 4) {
            char *buf = (char *)malloc(cap);
            if (!buf) break;
            if (_fullpath(buf, dir, cap)) {
                abs = buf;
                break;
            }
            free(buf);
        }
    }
#else
    char *abs = realpath(dir, NULL);
#endif
    if (!abs) return module;
    sal_path_trim_sep(abs);
    const char *base = sal_path_base(abs);
    const char *stem = module;
    if (base[0] && strcmp(base, ".") != 0 && strcmp(base, "..") != 0)
        stem = arena_strdup(a, base);
    free(abs);
    return stem;
}

const char *driver_page_stem(arena_t *a, const char *path)
{
    const char *module = sal_path_stem(a, path);
    if (strcmp(module, "salam") == 0) return "index";
    return module;
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
    sal_path_join(trimmed, sizeof trimmed, dir, "");
#if defined(_WIN32)
    char pattern[1100];
    sal_path_join(pattern, sizeof pattern, trimmed, "*.salam");
    struct _finddata_t fd;
    intptr_t h = _findfirst(is_cwd ? "*.salam" : pattern, &fd);
    if (h == -1) return;
    do {
        if (!(fd.attrib & _A_SUBDIR) && *n < SALAM_MAX_INPUTS)
            out[(*n)++] =
                is_cwd ? arena_strdup(a, fd.name) : sal_path_joina(a, trimmed, fd.name);
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(trimmed);
    if (!d) return;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && *n < SALAM_MAX_INPUTS) {
        size_t L = strlen(e->d_name);
        if (L > 6 && strcmp(e->d_name + L - 6, ".salam") == 0)
            out[(*n)++] = is_cwd ? arena_strdup(a, e->d_name)
                                 : sal_path_joina(a, trimmed, e->d_name);
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

/* Quiet lex+parse probe: does `path` contain a top-level `layout` block
 * (a front-end DSL page rather than a program)? */
static bool file_has_layout_block(arena_t *a, langpack_t *pack, const char *path)
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
                if (d->kind == AST_LAYOUT_BLOCK) {
                    found = true;
                    break;
                }
            }
        }
    }
    logger_free(quiet);
    return found;
}

/* Shared "resolve the entry file of `dir`" used by driver_build/driver_run/
 * driver_interp when no input was given (dir=".") or an explicit directory
 * was passed instead of a file. The fixed project entry file salam.salam
 * wins when present; otherwise the dir's top-level .salam files are scanned
 * for exactly one defining `entry`. Logs its own error and returns NULL on
 * failure; returns the resolved path (arena-owned) on success. */
const char *driver_resolve_dir_entry(arena_t *arena, logger_t *log, langpack_t *pack,
                                     const char *dir)
{
    const char *entry = langpack_entry(pack);
    const char *proj = driver_project_entry_file(arena, dir);
    if (proj) {
        if (!file_has_entry(arena, pack, entry, proj)) {
            LOG_E(log, PH_DRIVER,
                  i18n_tr("project entry file '%s' does not define a '%s' function"),
                  proj, entry);
            if (file_has_layout_block(arena, pack, proj))
                fprintf(stderr,
                        "  this looks like a web layout project; build it with: "
                        "salam web %s\n",
                        proj);
            return NULL;
        }
        LOG_I(log, PH_DRIVER, i18n_tr("entry point: %s"), proj);
        return proj;
    }
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
        fprintf(stderr, "  run a specific one with: salam run <file.salam>\n"
                        "  or name the project's entry file '" SALAM_PROJECT_FILE
                        "' to make it the project entry\n");
        return NULL;
    }
    LOG_I(log, PH_DRIVER, i18n_tr("entry point: %s"), entries[0]);
    return entries[0];
}

int driver_resolve_dir_layout(arena_t *arena, logger_t *log, langpack_t *pack,
                              const char *dir, const char **out, int max_out, bool single)
{
    const char *proj = driver_project_entry_file(arena, dir);
    if (proj) {
        LOG_I(log, PH_DRIVER, i18n_tr("entry point: %s"), proj);
        out[0] = proj;
        return 1;
    }
    const char *files[SALAM_MAX_INPUTS];
    int nfiles = 0;
    list_salam_files_in(arena, dir, files, &nfiles);
    if (nfiles == 0) {
        LOG_E(log, PH_DRIVER, i18n_tr("no .salam files found in the current directory"));
        return 0;
    }
    const char *pages[SALAM_MAX_INPUTS];
    int npages = 0;
    {
        int i = 0;
        for (; i < nfiles; i++)
            if (file_has_layout_block(arena, pack, files[i])) pages[npages++] = files[i];
    }
    if (npages == 0) {
        LOG_E(log, PH_DRIVER,
              i18n_tr("no layout entry: none of the %d .salam file%s here has a "
                      "'layout' block"),
              nfiles, plural_suffix(nfiles));
        fprintf(stderr, "  name the project's entry file '" SALAM_PROJECT_FILE
                        "' to make it the project entry\n");
        return 0;
    }
    if (single && npages > 1) {
        LOG_E(log, PH_DRIVER,
              i18n_tr("ambiguous layout entry: %d files have a "
                      "'layout' block:"),
              npages);
        int i = 0;
        for (; i < npages; i++)
            fprintf(stderr, "    %s\n", pages[i]);
        fprintf(stderr, "  build a specific one with: salam web <file.salam>\n"
                        "  or name the project's entry file '" SALAM_PROJECT_FILE
                        "' to make it the project entry\n");
        return 0;
    }
    int n = npages < max_out ? npages : max_out;
    {
        int i = 0;
        for (; i < n; i++)
            out[i] = pages[i];
    }
    return n;
}

/*
 * Backend choice for a native build.
 *
 * LLVM is preferred whenever this binary actually has it compiled in
 * (WITH_LLVM=1), because it optimizes and, with WITH_LLD=1 plus an embedded
 * sysroot, links in-process - no compiler or linker needed on the machine
 * running salam. A binary built without LLVM has nothing to switch to and
 * stays on the C backend.
 *
 * The C backend is not deprecated by this: --backend=c selects it, and so
 * does any explicit --cc=, which names a C compiler and therefore can only
 * mean the C path.
 */
/*
 * Run a C compiler/linker command, falling back to a GCC-style @response
 * file when the command line is too long for the platform to spawn.
 *
 * Windows caps a process command line at ~32 KB, and a link line naming one
 * object per module blows through that well before the module count becomes
 * unreasonable - a compiler laid out as 150+ files hits it, reported only as
 * the shell's "The command line is too long." Both gcc and tcc accept
 * @file, one argument per line.
 *
 * The threshold is deliberately below the real limit: `system()` goes
 * through the shell, which needs headroom of its own.
 */
#define SALAM_CMDLINE_SAFE 30000

static int run_cc_cmd(logger_t *log, const char *cmd, const char *builddir)
{
    size_t len = strlen(cmd);
    const char *sp;
    char rsp[1200];
    FILE *f;
    int rc;
    if (len < SALAM_CMDLINE_SAFE || !builddir) return system(cmd);

    /* Split at the first space: the program stays on the command line, every
     * argument after it moves into the response file. */
    sp = strchr(cmd, ' ');
    if (!sp) return system(cmd);

    sal_snprintf(rsp, sizeof rsp, "%s/link.rsp", builddir);
    f = fopen(rsp, "wb");
    if (!f) {
        LOG_W(log, PH_DRIVER, "cannot write %s; passing the full command line", rsp);
        return system(cmd);
    }
    {
        /* One argument per line. Arguments were already shell-quoted into
         * `cmd`; a response file needs no shell quoting, but keeping the
         * quotes is harmless to both gcc and tcc and preserves paths with
         * spaces. */
        const char *p = sp + 1;
        int inq = 0;
        for (; *p; p++) {
            if (*p == '"') inq = !inq;
            if (*p == ' ' && !inq)
                fputc('\n', f);
            else
                fputc(*p, f);
        }
        fputc('\n', f);
    }
    fclose(f);
    {
        sb_t rc_cmd;
        const char *q = cmd;
        sb_init(&rc_cmd);
        for (; q < sp; q++)
            sb_putc(&rc_cmd, *q);
        sb_puts(&rc_cmd, " @");
        sb_put_shell_arg(&rc_cmd, rsp);
        LOG_I(log, PH_DRIVER, "command line too long (%lu bytes); using %s",
              (unsigned long)len, rsp);
        rc = system(sb_cstr(&rc_cmd));
        sb_free(&rc_cmd);
    }
    return rc;
}

/* The C backend's half of `--backend=auto`: the best C compiler this host
 * has, preferring one shipped inside the install tree over one on $PATH at
 * each rung. clang leads because it is the same codegen family as the LLVM
 * backend, so an auto build degrades to the closest thing available rather
 * than to a different toolchain; tcc is the floor. */
/*
 * clang > gcc > tcc, but the *whole* ladder is walked over the bundled
 * install tree before $PATH is consulted at all - not bundled-then-$PATH
 * per rung. Interleaving them let a host gcc outrank the tcc a
 * self-contained install ships with, which is not the toolchain that
 * install was built and tested against: on Windows it swaps in a
 * mingw-w64 gcc whose <stdlib.h> spells __argv as a macro over
 * __p___argv(), so std/os/process' `extern void* __argv` becomes a
 * conflicting redeclaration and every program importing it stops
 * compiling. The $PATH pass still exists for the case it was added for -
 * an install that ships no toolchain at all, most notably a plain source
 * checkout.
 */
static bool resolve_auto_cc(char *out, size_t n, bool *bundled)
{
    static const char *const LADDER[] = {"clang", "gcc", "tcc"};
    size_t i = 0;
    for (; i < sizeof LADDER / sizeof LADDER[0]; i++) {
        if (salam_find_bundled_tool(LADDER[i], out, n)) {
            *bundled = true;
            return true;
        }
    }
    *bundled = false;
    for (i = 0; i < sizeof LADDER / sizeof LADDER[0]; i++)
        if (salam_find_path_tool(LADDER[i], out, n)) return true;
    return false;
}

/*
 * A bare tool name is one we are expected to look up: no directory
 * component and no flags. Anything else - "gcc -O2", "C:/tools/tcc.exe" -
 * is a command prefix the caller wrote deliberately and is passed through
 * untouched.
 */
static bool cc_is_bare_name(const char *cc)
{
    return cc && cc[0] && !strchr(cc, ' ') && !strchr(cc, '/') && !strchr(cc, '\\');
}

/*
 * Locate one named compiler: bundled install tree first, then $PATH. Same
 * precedence as resolve_auto_cc, minus the ladder - an explicit --cc says
 * *which* compiler, so only *where it lives* is still an open question.
 *
 * Honoring "an explicit --cc is a choice" by skipping resolution entirely
 * left the bare string for the shell to find, and the bundled toolchain is
 * not on $PATH: on Windows the tcc a release bundles sits in
 * <install>/tcc/tcc.exe, so `--cc=tcc` died with cmd.exe's "'tcc' is not
 * recognized" across every db/ and opencv/ test, which pass --cc=$DBCC.
 * Choosing the compiler and finding it are separate questions.
 */
static bool resolve_named_cc(const char *name, char *out, size_t n, bool *bundled)
{
    if (salam_find_bundled_tool(name, out, n)) {
        *bundled = true;
        return true;
    }
    *bundled = false;
    return salam_find_path_tool(name, out, n);
}

/*
 * SALAM_C_STRICT=1 makes any C-compiler diagnostic about generated code
 * fatal. An environment variable rather than a CLI flag because the callers
 * that need it are build scripts and CI, which set it once for a whole run
 * and do not thread a flag through every salam invocation they make.
 */
bool salam_c_strict(void)
{
    const char *v = getenv("SALAM_C_STRICT");
    return v && v[0] == '1' && v[1] == '\0';
}

static bool build_use_llvm(const options_t *opt)
{
    if (!strcmp(opt->backend, "c")) return false;
    if (!strcmp(opt->backend, "llvm")) return true;
    return salam_llvm_native_available() != 0;
}

int driver_build(options_t *opt)
{
    /* A bare `salam build` (no inputs) or a directory passed as the sole
     * input (`salam build .`, `salam build ../project/`) doesn't name a
     * file - resolve it to the project's entry file (salam.salam when
     * present, else the one .salam file defining `main`) BEFORE picking
     * a backend: the LLVM path reads opt->input directly and must see
     * the resolved file, not a directory, and the project-file rule has
     * to hold for every backend equally. */
    if (opt->input_count == 0 ||
        (opt->input_count == 1 && driver_path_is_dir(opt->inputs[0]))) {
        static char resolved[1024];
        logger_t *rlog = logger_new(stderr, opt->log_level, resolve_color(opt->color));
        arena_t *rarena = arena_new(1 << 20);
        langpack_t *rpack = langpack_load(opt->lang);
        const char *first = NULL;
        if (!rpack)
            LOG_E(rlog, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
        else
            first = driver_resolve_dir_entry(rarena, rlog, rpack,
                                             opt->input_count ? opt->inputs[0] : ".");
        if (first) sal_snprintf(resolved, sizeof resolved, "%s", first);
        logger_free(rlog);
        arena_free(rarena);
        if (!first) return 2;
        opt->inputs[0] = resolved;
        opt->input_count = 1;
        opt->input = resolved;
    }

    if (opt->llvm_target && opt->llvm_target[0]) return driver_llvm_build(opt);

    if (build_use_llvm(opt)) {
        int lrc = driver_llvm_build(opt);
        /*
         * SALAM_RC_LLVM_UNSUPPORTED means the module was emitted but is
         * incomplete - the program uses a construct the LLVM backend does
         * not lower yet. SALAM_RC_LLVM_LINK_FAILED means codegen was fine
         * but the in-process lld could not produce an executable, which is
         * what happens to every program that links a host shared library
         * (-lsqlite3, -lgtk-3): the bundled static musl sysroot has none of
         * them, while the C backend's host toolchain does. Falling back to
         * the C backend on both keeps every program that builds today
         * building, which is what makes defaulting to LLVM safe before
         * backend parity is finished. Any other non-zero code is a real
         * error (bad source) and is reported as-is rather than retried, so
         * a genuine mistake is not hidden behind a second compile.
         */
        if (lrc != SALAM_RC_LLVM_UNSUPPORTED && lrc != SALAM_RC_LLVM_LINK_FAILED)
            return lrc;
        if (!strcmp(opt->backend, "llvm")) return lrc;
        /* fputs, not fprintf: i18n_tr() returns a translated string, and a
         * translation carrying a '%' would be read as a format directive
         * with no argument behind it (clang -Wformat-security). The selfhost
         * driver prints this with EPrintln for the same reason. */
        fputs(i18n_tr("salam: falling back to the C backend for this file "
                      "(build with --backend=llvm to make this an error)\n"),
              stderr);
    }

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
    salam_set_stdlib_root(opt->stdlib_path);

#if !defined(_WIN32)
    bundled_musl_tcc_t musl_tcc;
    musl_tcc.active = false;
#endif
    /* `--backend=auto` is a priority ladder: in-process LLVM first (handled
     * by build_use_llvm above), else the C backend, which resolves its
     * compiler clang > gcc > tcc. clang leads because it is the same
     * codegen family as the LLVM backend, so an auto build degrades to the
     * closest thing available rather than to a different toolchain.
     *
     * The bundled install tree is searched first at every rung, then $PATH.
     * Without the $PATH half the ladder collapsed on any install that ships
     * no toolchain (a plain source checkout, most notably): nothing matched,
     * opt->cc kept its "nobody passed --cc" sentinel value - the literal
     * string "tcc" - and the build shelled out to a tcc that need not be
     * installed, on a host that may well have clang or gcc right there.
     *
     * opt->cc_explicit is what separates that sentinel from a user who
     * actually typed --cc=tcc. Testing the string alone made the two
     * identical, so an explicit --cc=tcc was silently resolved away to
     * whatever clang/gcc the ladder found first - which is not a compiler
     * the caller asked for, and on Windows swaps in a gcc that miscompiles
     * several stdlib paths. An explicit --cc is a choice, not a hint.
     *
     * It still has to be *found*, though: see resolve_named_cc. Only the
     * ladder is skipped for an explicit --cc, never the lookup. */
    /*
     * SALAM_CC names the C compiler for a caller that drives many builds at
     * once and cannot thread --cc through each of them - the test suite
     * holding the whole corpus to tcc and then to gcc is the case this
     * exists for. A --cc actually typed on the command line still wins; this
     * only replaces the "nobody chose" default, and is then resolved through
     * the same bundled-first lookup an explicit --cc gets.
     */
    {
        const char *env_cc = getenv("SALAM_CC");
        if (env_cc && env_cc[0] && !opt->cc_explicit) {
            opt->cc = env_cc;
            opt->cc_explicit = true;
        }
    }
    {
        static char raw_cc[1200], quoted_cc[1208];
        bool bundled = false, resolved = false;
        if (opt->cc && !opt->cc_explicit && strcmp(opt->cc, "tcc") == 0)
            resolved = resolve_auto_cc(raw_cc, sizeof raw_cc, &bundled);
        else if (opt->cc && opt->cc_explicit && cc_is_bare_name(opt->cc))
            resolved = resolve_named_cc(opt->cc, raw_cc, sizeof raw_cc, &bundled);
        if (resolved) {
            LOG_I(log, PH_DRIVER, "using %s C compiler: %s", bundled ? "bundled" : "host",
                  raw_cc);
#if !defined(_WIN32)
            if (bundled) {
                musl_tcc = detect_bundled_musl_tcc(raw_cc);
                if (musl_tcc.active)
                    LOG_I(log, PH_DRIVER, "using bundled musl sysroot: %s",
                          musl_tcc.musl_dir);
            }
#endif
            /* A --cc= the user typed is a command prefix and may carry
             * flags, so it goes into the command line verbatim; a path we
             * resolved ourselves is just a path, and $PATH entries like
             * "C:/Program Files/..." have to be quoted to survive it. */
            if (strchr(raw_cc, ' ')) {
                sal_snprintf(quoted_cc, sizeof quoted_cc, "\"%s\"", raw_cc);
                opt->cc = quoted_cc;
            } else {
                opt->cc = raw_cc;
            }
        }
    }

    const char *scratch = salam_scratch_dir();
    const char *cfiles[SALAM_MAX_INPUTS];
    /* Source path each generated .c came from, so --time-report can charge the
       host C compiler's time to the .salam file the user actually wrote. */
    const char *csrc[SALAM_MAX_INPUTS];
    int ncfiles = 0;
    /* Per-module object path, and whether that object must be rebuilt. A module
       is clean only when its regenerated .c/.h are byte-identical to what is
       already on disk, its .o exists, and the compile command has not changed. */
    const char *ofiles[SALAM_MAX_INPUTS];
    bool cdirty[SALAM_MAX_INPUTS];
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
    /* Shared across every work-list file so each imported package is loaded,
       parsed and analyzed once per build instead of once per importer. */
    vec_t pkg_cache;
    vec_init(&pkg_cache);

    dce_reset();
    dce_enable();

    /* --export=Fn:CName is split here but registered later, inside the module
       loop, because cg_add_export_override wants the DEFINING file's package
       and that is only known once the file has been parsed. */
    const char *export_fn[SALAM_MAX_INPUTS];
    const char *export_cname[SALAM_MAX_INPUTS];
    int nexport = 0;
    cg_reset_export_overrides();
    {
        int ei = 0;
        for (; ei < opt->nexports; ei++) {
            const char *spec = opt->exports[ei];
            const char *colon = strchr(spec, ':');
            if (!colon || colon == spec || colon[1] == '\0') {
                LOG_E(log, PH_DRIVER, "invalid --export value '%s' (want Fn:CName)",
                      spec);
                logger_free(log);
                arena_free(arena);
                return 2;
            }
            {
                size_t nlen = (size_t)(colon - spec);
                char *fn = (char *)arena_alloc(arena, nlen + 1);
                memcpy(fn, spec, nlen);
                fn[nlen] = '\0';
                export_fn[nexport] = fn;
                export_cname[nexport] = colon + 1;
                nexport++;
            }
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
            PROF_SCOPE_BEGIN(TP_SOURCE, path);
            source_file_t *src = source_load(arena, path);
            PROF_SCOPE_END(TP_SOURCE);
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
            const char *module = sal_path_stem(arena, path);
            if (!first_module) first_module = module;
            LOG_I(log, PH_DRIVER, "compiling %s -> %s.c", path, module);
            token_stream_t *toks = NULL;
            PROF_SCOPE_BEGIN(TP_LEXER, path);
            bool lok = lexer_run(arena, log, modpack, src, &toks);
            PROF_SCOPE_END(TP_LEXER);
            prof_self_count(TC_TOKENS, (uint64_t)token_stream_count(toks));
            ast_node_t *program = NULL;
            PROF_SCOPE_BEGIN(TP_PARSER, path);
            bool pok = parser_run(arena, log, toks, &program);
            if (!cc_prune_program(arena, log, path, cc, program)) pok = false;
            PROF_SCOPE_END(TP_PARSER);

            {
                const char *pfiles[SALAM_MAX_INPUTS];
                int npf = salam_package_files(arena, path, pfiles, SALAM_MAX_INPUTS);
                int pi = 1;
                for (; pi < npf; pi++) {
                    PROF_SCOPE_BEGIN(TP_SOURCE, pfiles[pi]);
                    source_file_t *psrc = source_load(arena, pfiles[pi]);
                    PROF_SCOPE_END(TP_SOURCE);
                    if (!psrc) {
                        LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"),
                              pfiles[pi]);
                        all_ok = false;
                        continue;
                    }
                    logger_add_diag_source(log, pfiles[pi], psrc->text, psrc->len);
                    token_stream_t *ptoks = NULL;
                    PROF_SCOPE_BEGIN(TP_LEXER, pfiles[pi]);
                    if (!lexer_run(arena, log, modpack, psrc, &ptoks)) lok = false;
                    PROF_SCOPE_END(TP_LEXER);
                    prof_self_count(TC_TOKENS, (uint64_t)token_stream_count(ptoks));
                    ast_node_t *pprog = NULL;
                    PROF_SCOPE_BEGIN(TP_PARSER, pfiles[pi]);
                    if (!parser_run(arena, log, ptoks, &pprog)) pok = false;
                    if (!cc_prune_program(arena, log, pfiles[pi], cc, pprog)) pok = false;
                    PROF_SCOPE_END(TP_PARSER);
                    salam_merge_program(arena, program, pprog);
                }
            }

            PROF_SCOPE_BEGIN(TP_SEMANTIC, path);
            sema_result_t *sr = sema_run_cached(arena, log, program, src->path,
                                                langpack_code(modpack), cc, &pkg_cache);
            PROF_SCOPE_END(TP_SEMANTIC);
            if (sr && sr->global)
                prof_self_count(TC_SYMBOLS, (uint64_t)sr->global->symbols.len);
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
            /* --export applies to the entry file only (wi == 0). Matching by
               name across every transitively imported module would catch
               unrelated same-named functions too. */
            if (wi == 0 && nexport > 0) {
                size_t k = 0;
                for (; k < program->list.len; k++) {
                    ast_node_t *d = (ast_node_t *)program->list.data[k];
                    if (d->kind != AST_FUNC_DEF || !d->name) continue;
                    {
                        int ei = 0;
                        for (; ei < nexport; ei++) {
                            if (strcmp(d->name, export_fn[ei]) != 0) continue;
                            dce_mark_root(modpkg, d->name);
                            cg_add_export_override(modpkg, d->name, export_cname[ei]);
                        }
                    }
                }
            }

            const char *idir = sal_path_dir(arena, path);
            {
                size_t k = 0;
                /* Deliberately no `nwork < SALAM_MAX_INPUTS` guard on the loop
                   itself. It ended the scan the moment work[] filled, which
                   dropped every remaining `link` directive in this file and
                   made the overflow report below unreachable - the body only
                   runs while nwork is still under the limit, so its
                   `nwork >= SALAM_MAX_INPUTS` arm could never be taken and the
                   closure truncated as silently as it did before that report
                   was added. Every push inside bounds-checks itself. */
                for (; k < program->list.len; k++) {
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
                    /* Every other push site bounds-checks; this one discovers
                       imports and is the one that actually grows the list, so
                       an import closure over SALAM_MAX_INPUTS files would run
                       off the end of work[] rather than report a limit. */
                    if (!known) {
                        if (nwork >= SALAM_MAX_INPUTS) {
                            LOG_E(log, PH_DRIVER,
                                  "too many modules in the import graph (limit %d)",
                                  SALAM_MAX_INPUTS);
                            all_ok = false;
                            break;
                        }
                        work[nwork++] = ipath;
                    }
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

            PROF_SCOPE_BEGIN(TP_CODEGEN, b_srcpath[wi]);
            codegen_output_t *out =
                codegen_run(arena, log, program, sr, module, opt->safe, opt->debug_info,
                            b_srcpath[wi], modentry, opt->llvm_target);
            PROF_SCOPE_END(TP_CODEGEN);
            if (prof_self_on()) {
                size_t fi = 0;
                uint64_t nfuncs = 0;
                for (; fi < program->list.len; fi++)
                    if (((ast_node_t *)program->list.data[fi])->kind == AST_FUNC_DEF)
                        nfuncs++;
                prof_self_count(TC_FUNCS_EMITTED, nfuncs);
            }
            size_t pfxlen = strlen(SALAM_MOD_PREFIX);
            size_t pathcap = strlen(scratch) + 1 + pfxlen + strlen(module) + 3;
            char *cpath = (char *)arena_alloc(arena, pathcap);
            char *hpath = (char *)arena_alloc(arena, pathcap);
            sal_snprintf(cpath, pathcap, "%s/%s%s.c", scratch, SALAM_MOD_PREFIX, module);
            sal_snprintf(hpath, pathcap, "%s/%s%s.h", scratch, SALAM_MOD_PREFIX, module);
            {
                char *opath = (char *)arena_alloc(arena, pathcap);
                bool unchanged;
                sal_snprintf(opath, pathcap, "%s/%s%s.o", scratch, SALAM_MOD_PREFIX,
                             module);
                PROF_SCOPE_BEGIN(TP_WRITE, b_srcpath[wi]);
                /* Compare before writing: rewriting an identical file would only
                   churn its timestamp and force a needless recompile. */
                unchanged = !opt->force && file_exists(opath) &&
                            file_has_content(cpath, out->c_src) &&
                            file_has_content(hpath, out->h_src);
                if (!unchanged && (!write_file(log, cpath, out->c_src) ||
                                   !write_file(log, hpath, out->h_src))) {
                    PROF_SCOPE_END(TP_WRITE);
                    all_ok = false;
                    continue;
                }
                PROF_SCOPE_END(TP_WRITE);
                ofiles[ncfiles] = opath;
                cdirty[ncfiles] = !unchanged;
            }
            csrc[ncfiles] = b_srcpath[wi] ? b_srcpath[wi] : cpath;
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
    /*
     * -fno-strict-aliasing is not optional, and is deliberately applied
     * whenever the compiler is not tcc - independently of -O, so a caller
     * who passes their own --cc="gcc -O2" still gets it.
     *
     * Generated C reaches every aggregate through void*: slices hand back
     * `void*` from salam_slice_at that the caller casts to the element
     * type, Vector payloads are re-cast per element, and str arrays are
     * walked as int64_t*. That is exactly the type punning C's strict
     * aliasing rules forbid, so at -O2 gcc is free to assume those accesses
     * cannot alias and reorders them - `fmt.Sprintf` over a Vector<str>
     * segfaulted, and a dozen tests silently truncated their output
     * mid-run. -O0 and -O1 were fine, which is what made it look like a
     * miscompile rather than UB in what we emit. Every compiler that emits
     * C this way disables the assumption; the real alternative is emitting
     * unions or char* accessors everywhere, which this backend does not do.
     */
    const char *alias_flag = strstr(opt->cc, "tcc") ? "" : " -fno-strict-aliasing";
    /*
     * Strict mode: every diagnostic the C compiler would have printed about
     * generated code becomes a hard error, so a codegen regression stops the
     * build instead of scrolling past.
     *
     * -Werror on its own, deliberately: it promotes what the compiler
     * already decided to warn about rather than switching on new warning
     * classes. Generated C is not hand-written C - it is entitled to look
     * odd - so the bar being enforced is "the toolchain found nothing wrong
     * with it", which is stable across tcc, gcc and clang. Adding -Wall
     * here would instead import each compiler's opinion about style and
     * make the gate mean something different on every host.
     *
     * Off unless asked for. A user's newer gcc inventing a warning must not
     * break their build of a release that predates it; the test suite turns
     * it on (see run-tests.sh) because that is where a regression has to be
     * caught, and where the compiler set is known.
     *
     * Two diagnostics are switched off rather than promoted, because neither
     * can tell what the generated code means:
     *
     *   div-by-zero            fires on a constant-folded `10 / d` sitting in
     *                          the right arm of `d != 0 && ...`. Salam's &&
     *                          short-circuits, so that arm never runs, but the
     *                          fold happens before the compiler knows that.
     *                          Silencing it costs nothing: a division by a
     *                          literal zero that IS reachable is a Salam-level
     *                          bug the semantic pass should catch, not
     *                          something to discover through gcc.
     *   deprecated-declarations
     *                          codegen puts __attribute__((deprecated)) on the
     *                          prototype so hand-written C including the
     *                          header is warned. Salam call sites already get
     *                          the semantic pass's own deprecation warning -
     *                          which is what tests/en/general/deprecated_chain
     *                          asserts - so promoting the C copy of it turns
     *                          "this test exercises a deprecated function" into
     *                          a build failure.
     */
    const char *werror_flag =
        salam_c_strict() ? " -Werror -Wno-div-by-zero -Wno-deprecated-declarations" : "";
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
                sb_puts(&cmd, alias_flag);
                sb_puts(&cmd, werror_flag);
                sb_puts(&cmd, dbg_flag);
                sb_putc(&cmd, ' ');
                sb_put_shell_arg(&cmd, cfiles[i]);
                sb_puts(&cmd, " -o ");
                sb_put_shell_arg(&cmd, obj);
                LOG_I(log, PH_DRIVER, "assembling: %s", sb_cstr(&cmd));
                PROF_SCOPE_BEGIN(TP_CC, csrc[i]);
                crc = system(sb_cstr(&cmd));
                PROF_SCOPE_END(TP_CC);
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
            const char *stem = opt->input_count > 0
                                   ? driver_output_stem(arena, opt->inputs[0])
                                   : first_module;
            size_t ocap = strlen(stem) + 5;
            char *o = (char *)arena_alloc(arena, ocap);
            sal_snprintf(o, ocap, "%s.exe", stem);
            output = o;
        }
#ifdef _WIN32
        const char *lm = use_tcc ? " -lmsvcrt" : (strstr(opt->cc, "clang") ? "" : " -lm");
#else

        const char *lm = " -lm";
#endif
        /* Opt-in via --lto, not implied by an optimizing build. -flto turns
         * every per-module .o into bitcode, so the object cache below stops
         * buying anything and the link becomes one serial LTRANS pass over
         * the whole program; on Windows the toolchain's lto-wrapper falls
         * back to serial LTRANS regardless of -flto=N anyway, and mingw's
         * lto1 fails outright on some hosts. The parallel per-file compile
         * is the better default, so LTO is a choice the caller makes. */
        const char *lto_flag = "";
        if (opt->lto && optimizing_build && !use_tcc) {
#ifdef _WIN32
            if (!strstr(opt->cc, "clang")) lto_flag = " -flto";
#else
            lto_flag = " -flto";
#endif
        }
        char hostlibs[1024];
        hostlibs[0] = '\0';

        /* AddressSanitizer is rejected before anything is compiled, not midway
           through, so a failed build leaves no half-written object cache. */
        if (opt->asan && use_tcc) {
            LOG_E(log, PH_DRIVER,
                  "tcc does not support AddressSanitizer; use --cc=gcc or --cc=clang");
            if (!opt->keep_c) {
                int i = 0;
                for (; i < ngen; i++)
                    remove(generated[i]);
            }
            logger_free(log);
            arena_free(arena);
            return 2;
        }

        /* Compile each module to its own .o, skipping those whose generated C
           was byte-identical to the cached copy. The flags below must match the
           ones the link command uses, so they are rebuilt from the same values. */
        {
            sb_t cflags;
            sb_init(&cflags);
#if !defined(_WIN32)
            if (musl_tcc.active) {
                sb_puts(&cflags, " -B");
                sb_put_shell_arg(&cflags, musl_tcc.tcc_dir);
                sb_puts(&cflags, " -nostdlib -static -I");
                sb_put_shell_arg(&cflags, musl_tcc.tcc_dir);
                sb_puts(&cflags, "/include -I");
                sb_put_shell_arg(&cflags, musl_tcc.musl_dir);
                sb_puts(&cflags, "/include");
            }
#endif
            sb_puts(&cflags, opt_flag);
            sb_puts(&cflags, alias_flag);
            sb_puts(&cflags, werror_flag);
            sb_puts(&cflags, lto_flag);
            sb_puts(&cflags, " -I. -I");
            sb_put_shell_arg(&cflags, scratch);
            if (opt->debug_info && !use_tcc) sb_puts(&cflags, " -g");
            if (opt->asan)
                sb_puts(&cflags,
                        " -fsanitize=address -fno-omit-frame-pointer -DSALAM_MEM_DEBUG");
            {
                int i = 0;
                for (; i < nlinks; i++) {
                    sb_puts(&cflags, " -DSALAM_LINK_");
                    {
                        const char *p = links[i];
                        for (; *p; p++) {
                            char c = *p;
                            if (c >= 'a' && c <= 'z')
                                c = (char)(c - 'a' + 'A');
                            else if (!((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')))
                                c = '_';
                            sb_putc(&cflags, c);
                        }
                    }
                }
            }

            /* Cached objects were built with the previous flag set; if it moved
               (--release, -g, --asan, a new link directive, a different cc) every
               object is stale regardless of whether its source changed. */
            size_t sigcap = strlen(scratch) + 32;
            char *sigpath = (char *)arena_alloc(arena, sigcap);
            sb_t sig;
            bool sig_pending = false;
            sb_init(&sig);
            sb_puts(&sig, opt->cc);
            sb_puts(&sig, sb_cstr(&cflags));
            /* --export renames symbols in the emitted C, so an object built
               under a different set of them is stale even though its own
               source did not change. */
            {
                int ei = 0;
                for (; ei < opt->nexports; ei++) {
                    sb_puts(&sig, "|E");
                    sb_puts(&sig, opt->exports[ei]);
                }
            }
            sal_snprintf(sigpath, sigcap, "%s/salam_build.flags", scratch);
            if (!file_has_content(sigpath, sb_cstr(&sig))) {
                int i = 0;
                for (; i < ncfiles; i++)
                    cdirty[i] = true;
                /*
                 * Recorded only once every object has actually been rebuilt
                 * with these flags - see after the compile loop. Writing it
                 * here, before compiling, meant a build that failed or was
                 * interrupted left the file claiming a flag set the objects
                 * on disk were never built with. The next run then believed
                 * its cache and linked whatever was lying there: switching
                 * gcc -> tcc that way fed the linker gcc's objects and it
                 * died on "salam_mod_mem.o:1: error: unrecognized file
                 * type", a message that points at nothing the user did.
                 */
                sig_pending = true;
            }

            {
                int i = 0;
                int nrebuilt = 0;
                for (; i < ncfiles && crc == 0; i++) {
                    sb_t cc1;
                    if (!cdirty[i]) continue;
                    sb_init(&cc1);
                    sb_puts(&cc1, opt->cc);
                    sb_puts(&cc1, sb_cstr(&cflags));
                    sb_puts(&cc1, " -c ");
                    sb_put_shell_arg(&cc1, cfiles[i]);
                    sb_puts(&cc1, " -o ");
                    sb_put_shell_arg(&cc1, ofiles[i]);
                    LOG_I(log, PH_DRIVER, "compiling: %s", sb_cstr(&cc1));
                    PROF_SCOPE_BEGIN(TP_CC, csrc[i]);
                    crc = system(sb_cstr(&cc1));
                    PROF_SCOPE_END(TP_CC);
                    sb_free(&cc1);
                    nrebuilt++;
                }
                LOG_I(log, PH_DRIVER, "compiled %d module(s), reused %d cached object(s)",
                      nrebuilt, ncfiles - nrebuilt);
                prof_self_count(TC_MODULES_BUILT, (uint64_t)nrebuilt);
                prof_self_count(TC_MODULES_CACHED, (uint64_t)(ncfiles - nrebuilt));
            }
            /* Every object now matches these flags, so the cache may claim
               them. On failure the file is deliberately left alone: the next
               run must still see the old signature, mark everything dirty
               and rebuild rather than trust half-written state. */
            if (crc == 0 && sig_pending) write_file(log, sigpath, sb_cstr(&sig));
            sb_free(&sig);
            sb_free(&cflags);
        }
        if (crc != 0) {
            LOG_E(log, PH_DRIVER, i18n_tr("C compiler '%s' failed (exit %d)"), opt->cc,
                  crc);
            if (!opt->keep_c) {
                int i = 0;
                for (; i < ngen; i++)
                    remove(generated[i]);
            }
            logger_free(log);
            arena_free(arena);
            return 3;
        }

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
        sb_puts(&cmd, alias_flag);
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
        /* The tcc rejection already happened before the compile phase. */
        if (opt->asan) sb_puts(&cmd, " -fsanitize=address -fno-omit-frame-pointer");

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
                sb_put_shell_arg(&cmd, ofiles[i]);
            }
        }
        sb_puts(&cmd, lm);

        if (try_embed_hostlibs(log, hostlibs, sizeof hostlibs)) {
            sb_puts(&cmd, " -L");
            sb_put_shell_arg(&cmd, hostlibs);
        }
        /* --libpath=DIR, same as the LLVM link paths honor - so an archive
         * that is not installed yet (a freshly built libsalam_llvm.a, say)
         * is linkable through the C backend too, not only through LLVM. */
        {
            int i = 0;
            for (; i < opt->nlibpath; i++) {
                sb_puts(&cmd, " -L");
                sb_put_shell_arg(&cmd, opt->lib_paths[i]);
            }
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
        PROF_SCOPE_BEGIN(TP_LINK, output);
        crc = run_cc_cmd(log, sb_cstr(&cmd), salam_scratch_dir());
        PROF_SCOPE_END(TP_LINK);
        sb_free(&cmd);
        if (crc != 0) {
            LOG_E(log, PH_DRIVER, i18n_tr("C compiler '%s' failed (exit %d)"), opt->cc,
                  crc);
            rc = 3;
        } else {
            LOG_I(log, PH_DRIVER, "built executable: %s", output);
            copy_hostlib_shared_libs(log, arena, hostlibs, output, links, nlinks);

            if (opt->exe_path[0] == '\0')
                sal_snprintf(opt->exe_path, sizeof(opt->exe_path), "%s", output);
        }
    }
    /* The generated .c/.h are the object cache's key: deleting them would make
       every module look changed on the next build. They stay in the scratch
       dir (gitignored) and --force ignores them anyway. */
    if (opt->keep_c) LOG_I(log, PH_DRIVER, "kept generated C files");
    {
        arena_stats_t as = arena_stats(arena);
        prof_self_count(TC_ARENA_BYTES, as.bytes_reserved);
        prof_self_count(TC_ARENA_BLOCKS, as.blocks);
        prof_self_count(TC_AST_NODES, ast_node_count());
    }
    logger_free(log);
    arena_free(arena);
    return rc;
}
