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

/*
 * `salam doc [path]` - render API documentation for a file, a package, or a
 * whole source tree as one self-contained HTML page.
 *
 * This is the command wiring only; the lexing/parsing/AST-walking and the HTML
 * rendering all live in doc/docgen.c. File discovery mirrors `salam format`:
 * a directory input is walked recursively, while a single file input pulls in
 * its package siblings through salam_package_files() so that documenting
 * std/str/str.salam documents the whole std/str package.
 *
 * Ported from the self-hosted compiler/cli.salam's doc_cmd; keep the two in
 * sync.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "core/sb.h"
#include "core/arena.h"
#include "core/vec.h"
#include "driver/doc_build.h"
#include "driver/driver.h"
#include "doc/docgen.h"
#include "langpack/langpack.h"
#include "semantic/sema.h"
#include "i18n/i18n.h"
#include <errno.h>
#include <sys/stat.h>
#if defined(_WIN32)
#  include <io.h>
#else
#  include <dirent.h>
#endif

#define DOC_PATH_MAX 1024

static const char *plural_suffix(int n)
{
    return n == 1 ? "" : "s";
}

static bool has_salam_ext(const char *name)
{
    size_t L = strlen(name);
    return L > 6 && strcmp(name + L - 6, ".salam") == 0;
}

static bool path_is_dir(const char *p)
{
#if defined(_WIN32)
    struct _stat st;
    return _stat(p, &st) == 0 && (st.st_mode & _S_IFDIR) != 0;
#else
    struct stat st;
    return stat(p, &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR;
#endif
}

static bool path_exists(const char *p)
{
#if defined(_WIN32)
    struct _stat st;
    return _stat(p, &st) == 0;
#else
    struct stat st;
    return stat(p, &st) == 0;
#endif
}

/* "src/" from "src/main.salam"; "" when there is no directory part. */
static void dir_prefix_of(const char *path, char *out, size_t cap)
{
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *cut = slash;
    size_t n;
    if (bslash && (!cut || bslash > cut)) cut = bslash;
    if (!cut) {
        out[0] = '\0';
        return;
    }
    n = (size_t)(cut - path) + 1;
    if (n >= cap) n = cap - 1;
    memcpy(out, path, n);
    out[n] = '\0';
}

static void path_stem(const char *path, char *out, size_t cap)
{
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *base = path;
    const char *dot;
    size_t len;
    if (slash && slash + 1 > base) base = slash + 1;
    if (bslash && bslash + 1 > base) base = bslash + 1;
    dot = strrchr(base, '.');
    len = dot ? (size_t)(dot - base) : strlen(base);
    if (len >= cap) len = cap - 1;
    memcpy(out, base, len);
    out[len] = '\0';
}

static void strip_trailing_slash(const char *p, char *out, size_t cap)
{
    size_t n = strlen(p);
    if (n > 0 && (p[n - 1] == '/' || p[n - 1] == '\\')) n--;
    if (n >= cap) n = cap - 1;
    memcpy(out, p, n);
    out[n] = '\0';
}

/*
 * Directory input -> "<dir>/doc.html", so a bare `salam doc` writes
 * "./doc.html"; single-file input -> "<same dir>/<stem>.doc.html".
 * --output=FILE overrides either default.
 */
static void doc_output_path(const options_t *opt, const char *path, char *out, size_t cap)
{
    if (opt->output && opt->output[0]) {
        sal_snprintf(out, cap, "%s", opt->output);
        return;
    }
    if (path_is_dir(path)) {
        char base[DOC_PATH_MAX];
        strip_trailing_slash(path, base, sizeof base);
        sal_snprintf(out, cap, "%s/doc.html", base);
    } else {
        char dir[DOC_PATH_MAX];
        char stem[DOC_PATH_MAX];
        dir_prefix_of(path, dir, sizeof dir);
        path_stem(path, stem, sizeof stem);
        sal_snprintf(out, cap, "%s%s.doc.html", dir, stem);
    }
}

static void doc_push_file(arena_t *a, vec_t *files, const char *path)
{
    vec_push(a, files, CONST_CAST(arena_strdup(a, path)));
}

static void doc_walk(arena_t *a, vec_t *files, const char *dir)
{
#if defined(_WIN32)
    char pattern[DOC_PATH_MAX];
    struct _finddata_t fd;
    intptr_t h;
    sal_snprintf(pattern, sizeof pattern, "%s/*", dir);
    h = _findfirst(pattern, &fd);
    if (h == -1) return;
    do {
        char child[DOC_PATH_MAX];
        if (fd.name[0] == '.') continue;
        sal_snprintf(child, sizeof child, "%s/%s", dir, fd.name);
        if (fd.attrib & _A_SUBDIR)
            doc_walk(a, files, child);
        else if (has_salam_ext(fd.name))
            doc_push_file(a, files, child);
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(dir);
    struct dirent *e;
    if (!d) return;
    while ((e = readdir(d)) != NULL) {
        char child[DOC_PATH_MAX];
        if (e->d_name[0] == '.') continue;
        sal_snprintf(child, sizeof child, "%s/%s", dir, e->d_name);
        if (path_is_dir(child))
            doc_walk(a, files, child);
        else if (has_salam_ext(e->d_name))
            doc_push_file(a, files, child);
    }
    closedir(d);
#endif
}

static bool doc_write(const char *path, const sb_t *html)
{
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, i18n_tr("salam: cannot write '%s': %s\n"), path, strerror(errno));
        return false;
    }
    fwrite(sb_cstr(html), 1, html->len, f);
    fclose(f);
    return true;
}

int driver_doc(options_t *opt)
{
    /* One path only (cli_options.c rejects a second one), defaulting to the
     * current directory so a bare `salam doc` documents the tree it is run in. */
    const char *path = opt->input ? opt->input : ".";
    arena_t *arena;
    langpack_t *pack;
    doc_db_t *db;
    vec_t files;
    sb_t html;
    char base[DOC_PATH_MAX];
    char title[DOC_PATH_MAX];
    char out[DOC_PATH_MAX];
    int warn = 0, pub_count = 0, priv_count = 0;
    size_t i;
    int rc;

    if (!path_exists(path)) {
        fprintf(stderr, i18n_tr("salam doc: '%s' does not exist\n"), path);
        return 2;
    }
    pack = langpack_load(opt->lang);
    if (!pack) {
        fprintf(stderr, i18n_tr("salam: unknown language pack '%s'\n"), opt->lang);
        return 2;
    }

    arena = arena_new(1 << 16);
    vec_init(&files);
    if (path_is_dir(path)) {
        strip_trailing_slash(path, base, sizeof base);
        doc_walk(arena, &files, base);
    } else {
        /* A single file documents its whole package: same sibling auto-glob
         * the compiler itself applies to imports. */
        const char *pf[SALAM_MAX_INPUTS];
        int npf = salam_package_files(arena, path, pf, SALAM_MAX_INPUTS);
        int k = 0;
        for (; k < npf; k++)
            doc_push_file(arena, &files, pf[k]);
    }
    if (files.len == 0) {
        fprintf(stderr, i18n_tr("salam doc: no .salam files found under '%s'\n"), path);
        arena_free(arena);
        langpack_free(pack);
        return 2;
    }

    db = docgen_db_new();
    for (i = 0; i < files.len; i++) {
        const char *p = (const char *)files.data[i];
        if (!docgen_parse_file(db, pack, p, p, opt->defines, opt->ndefines)) warn++;
    }

    strip_trailing_slash(path, base, sizeof base);
    path_stem(base, title, sizeof title);
    sb_init(&html);
    docgen_render(db, title, &html);
    doc_output_path(opt, path, out, sizeof out);

    rc = 0;
    if (!doc_write(out, &html)) {
        rc = 2;
    } else {
        for (i = 0; i < db->decls.len; i++) {
            if (((const doc_decl_t *)db->decls.data[i])->is_pub)
                pub_count++;
            else
                priv_count++;
        }
        printf("salam doc: scanned %d file%s (%d public, %d private declarations)\n",
               (int)files.len, plural_suffix((int)files.len), pub_count, priv_count);
        printf("  -> %s\n", out);
        if (warn > 0) {
            /* Without this, a nonzero exit and a few silently missing entries
             * are the only clue that a source file failed to parse. */
            printf("  note: %d file%s did not parse cleanly; declarations the parser "
                   "could not read were skipped\n",
                   warn, plural_suffix(warn));
            rc = 1;
        }
    }

    sb_free(&html);
    docgen_db_free(db);
    arena_free(arena);
    langpack_free(pack);
    return rc;
}
