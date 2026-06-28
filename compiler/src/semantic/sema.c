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
#include "core/numstr.h"
#include "semantic/sema.h"
#include "semantic/sema_internal.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "langpack/langpack.h"
#include "i18n/i18n.h"
#include "preproc/preproc.h"
#if defined(_WIN32)
#  include <io.h>
#  include <windows.h>
#else
#  include <dirent.h>
#  include <unistd.h>
#endif
#if defined(__APPLE__)
#  include <mach-o/dyld.h>
#endif

static char  g_stdlib_root_buf[1200];
static const char *g_stdlib_root = NULL;   /* resolved root, or "" for cwd */
static bool        g_stdlib_resolved = false;

static bool file_exists(const char *p);    /* fwd decl */

static bool root_has_std(const char *root)
{
    char p[1100];
    if (root && root[0]) snprintf(p, sizeof p, "%s/std/core/core.salam", root);
    else                 snprintf(p, sizeof p, "std/core/core.salam");
    return file_exists(p);
}

static void derive_root(const char *path, char *out, size_t n)
{
    char probe[1100];
    snprintf(probe, sizeof probe, "%s/std/core/core.salam", path);
    if (file_exists(probe)) { snprintf(out, n, "%s", path); return; }

    snprintf(probe, sizeof probe, "%s/core/core.salam", path);
    if (file_exists(probe)) {                 /* path is the std/ dir itself */
        snprintf(out, n, "%s", path);
        char *s = out + strlen(out);
        while (s > out && (s[-1] == '/' || s[-1] == '\\')) *--s = '\0';
        char *slash = strrchr(out, '/');
        char *bs    = strrchr(out, '\\');
        char *cut   = (bs && (!slash || bs > slash)) ? bs : slash;
        if (cut) *cut = '\0'; else out[0] = '\0';
        return;
    }
    snprintf(out, n, "%s", path);             /* trust as-is */
}

static bool get_exe_dir(char *out, size_t n)
{
    char buf[1024];
#if defined(_WIN32)
    DWORD len = GetModuleFileNameA(NULL, buf, (DWORD)sizeof buf);
    if (len == 0 || len >= sizeof buf) return false;
#elif defined(__APPLE__)
    uint32_t sz = (uint32_t)sizeof buf;
    if (_NSGetExecutablePath(buf, &sz) != 0) return false;
#elif defined(__linux__)
    ssize_t len = readlink("/proc/self/exe", buf, sizeof buf - 1);
    if (len <= 0) return false;
    buf[len] = '\0';
#else
    (void)buf; (void)out; (void)n; return false;
#endif
#if !defined(__linux__) && !defined(_WIN32) && !defined(__APPLE__)
    return false;
#else
    {
        char *slash = strrchr(buf, '/');
        char *bs    = strrchr(buf, '\\');
        char *cut   = (bs && (!slash || bs > slash)) ? bs : slash;
        if (!cut) return false;
        *cut = '\0';
        snprintf(out, n, "%s", buf);
        return true;
    }
#endif
}

static bool read_cfg_stdlib(const char *cfg, char *out, size_t n)
{
    FILE *f = fopen(cfg, "rb");
    if (!f) return false;
    char line[1024];
    bool found = false;
    while (fgets(line, sizeof line, f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';') continue;
        const char *key = NULL;
        if      (strncmp(p, "stdlib_path", 11) == 0) key = p + 11;
        else if (strncmp(p, "stdlib",      6)  == 0) key = p + 6;
        if (!key) continue;
        while (*key == ' ' || *key == '\t') key++;
        if (*key != '=') continue;
        key++;
        while (*key == ' ' || *key == '\t') key++;
        size_t len = strlen(key);
        while (len > 0 && (key[len-1] == '\n' || key[len-1] == '\r' ||
                           key[len-1] == ' '  || key[len-1] == '\t')) len--;
        if (len == 0 || len >= n) continue;
        memcpy(out, key, len); out[len] = '\0';
        found = true;
        break;
    }
    fclose(f);
    return found;
}

static const char *resolve_stdlib_root(const char *explicit_path)
{
    char *buf = g_stdlib_root_buf;
    size_t  N = sizeof g_stdlib_root_buf;

    if (explicit_path && explicit_path[0]) {
        derive_root(explicit_path, buf, N);
        return buf;
    }
    {
        const char *env = getenv("SALAM_STD");
        if (env && env[0]) { derive_root(env, buf, N); return buf; }
    }
    {
        char exedir[1024];
        if (get_exe_dir(exedir, sizeof exedir)) {
            /* 3. salam.cfg next to the binary */
            char cfg[1100], val[1024];
            snprintf(cfg, sizeof cfg, "%s/salam.cfg", exedir);
            if (read_cfg_stdlib(cfg, val, sizeof val)) {
                derive_root(val, buf, N);
                if (root_has_std(buf)) return buf;
            }
            /* 4. auto-discovery relative to the binary */
            {
                static const char *rel[] = {
                    ".",                /* binary sits next to std/ (in-repo / portable) */
                    "../share/salam",   /* PREFIX/bin + PREFIX/share/salam (make install) */
                    "../lib/salam",
                    ".."
                };
                size_t i = 0;
                for (; i < sizeof rel / sizeof rel[0]; i++) {
                    char cand[1200];
                    snprintf(cand, sizeof cand, "%s/%s", exedir, rel[i]);
                    if (root_has_std(cand)) { snprintf(buf, N, "%s", cand); return buf; }
                }
            }
        }
    }
#ifdef SALAM_STDLIB_PREFIX
    if (root_has_std(SALAM_STDLIB_PREFIX)) {
        snprintf(buf, N, "%s", SALAM_STDLIB_PREFIX);
        return buf;
    }
#endif
    return "";
}

void salam_set_stdlib_root(const char *root)
{
    g_stdlib_root     = resolve_stdlib_root(root);
    g_stdlib_resolved = true;
}

const char *salam_get_stdlib_root(void)
{
    if (!g_stdlib_resolved) salam_set_stdlib_root(NULL);
    return g_stdlib_root ? g_stdlib_root : "";
}

bool salam_find_bundled_tool(const char *name, char *out, size_t n)
{
    char exedir[1024];
    static const char *rel[] = { "", "bin", "mingw64/bin", "llvm/bin", "tcc" };
#if defined(_WIN32)
    static const char *suffix = ".exe";
#else
    static const char *suffix = "";
#endif
    size_t i;

    if (!name || !name[0]) return false;
    if (!get_exe_dir(exedir, sizeof exedir)) return false;

    for (i = 0; i < sizeof rel / sizeof rel[0]; i++) {
        char cand[1200];
        FILE *f;
        if (rel[i][0])
            snprintf(cand, sizeof cand, "%s/%s/%s%s", exedir, rel[i], name, suffix);
        else
            snprintf(cand, sizeof cand, "%s/%s%s", exedir, name, suffix);
        f = fopen(cand, "rb");
        if (f) { fclose(f); snprintf(out, n, "%s", cand); return true; }
    }
    return false;
}

static char *path_normalize(char *p)
{
    while (p[0] == '.' && p[1] == '/') memmove(p, p + 2, strlen(p + 2) + 1);

    char *q;
    while ((q = strstr(p, "/./")) != NULL) memmove(q + 1, q + 3, strlen(q + 3) + 1);

    /* Collapse dir/../ sequences to prevent directory traversal */
    while ((q = strstr(p, "/../")) != NULL) {
        char *s = q;
        while (s > p && s[-1] != '/') s--;
        memmove(s, q + 4, strlen(q + 4) + 1);
    }
    /* Strip leading ../ sequences */
    while (p[0] == '.' && p[1] == '.' && p[2] == '/') memmove(p, p + 3, strlen(p + 3) + 1);

    return p;
}

static bool file_exists(const char *p)
{
    FILE *f = fopen(p, "rb");
    if (f) { fclose(f); return true; }
    return false;
}

static int list_entries(arena_t *a, const char *dir, const char **out, int max)
{
    int n = 0;
#if defined(_WIN32)
    char pat[512]; snprintf(pat, sizeof pat, "%s/*", dir);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pat, &fd);
    if (h == -1) return 0;
    do {
        if (strcmp(fd.name, ".") && strcmp(fd.name, "..") && n < max)
            out[n++] = arena_strdup(a, fd.name);
    } while (_findnext(h, &fd) == 0);
    _findclose(h);
#else
    DIR *d = opendir(dir);
    if (!d) return 0;
    struct dirent *e;
    while ((e = readdir(d)) != NULL && n < max) {
        if (strcmp(e->d_name, ".") && strcmp(e->d_name, ".."))
            out[n++] = arena_strdup(a, e->d_name);
    }
    closedir(d);
#endif
    return n;
}

typedef struct { const char *alias; const char *path; } pkg_alias_t;

static pkg_alias_t g_pkg_aliases[128];

static int         g_pkg_alias_n = -1;   

static void index_pkg_file(arena_t *a, const char *path)
{
    source_file_t *src = source_load(a, path);
    if (!src) return;
    const char *txt = src->text;
    const char *pkg = strstr(txt, "package ");
    size_t limit = pkg ? (size_t)(pkg - txt) : src->len;
    { size_t i = 0; for (; i + 4 < limit && g_pkg_alias_n < 128; i++) {
        if (txt[i] != '@') continue;
        bool en = txt[i+1]=='e' && txt[i+2]=='n';
        bool fa = txt[i+1]=='f' && txt[i+2]=='a';
        if (!en && !fa) continue;
        const char *q = strchr(txt + i + 3, '"');           
        if (!q || (size_t)(q - txt) >= limit) continue;
        const char *e = strchr(q + 1, '"');                 
        if (!e) continue;
        g_pkg_aliases[g_pkg_alias_n].alias = arena_strndup(a, q + 1, (size_t)(e - q - 1));
        g_pkg_aliases[g_pkg_alias_n].path  = path;
        g_pkg_alias_n++;
        i = (size_t)(e - txt);                               
    } }
}

static void build_pkg_alias_index(arena_t *a, const char *root)
{
    g_pkg_alias_n = 0;
    char base[512];
    if (root && root[0]) snprintf(base, sizeof base, "%s/std", root);
    else                 snprintf(base, sizeof base, "std");
    const char *entries[256];
    int ne = list_entries(a, base, entries, 256);
    { int i = 0; for (; i < ne; i++) {
        
        size_t need = strlen(base) + 2 * strlen(entries[i]) + sizeof("//.salam");
        char *pf = (char *)arena_alloc(a, need);
        snprintf(pf, need, "%s/%s/%s.salam", base, entries[i], entries[i]);
        if (file_exists(pf)) index_pkg_file(a, pf);
    } }
}

static const char *resolve_pkg_alias(arena_t *a, const char *root, const char *spec)
{
    if (g_pkg_alias_n < 0) build_pkg_alias_index(a, root);
    { int i = 0; for (; i < g_pkg_alias_n; i++)
        if (strcmp(g_pkg_aliases[i].alias, spec) == 0) return g_pkg_aliases[i].path; }
    return NULL;
}

const char *salam_resolve_import(arena_t *a, const char *dir, const char *spec)
{
    const char *root = salam_get_stdlib_root();
    bool has_ext = strstr(spec, ".salam") != NULL;
    size_t n = strlen(root) + strlen(dir) + 2 * strlen(spec) + 16;
    char *p = (char *)arena_alloc(a, n);
    
    if (has_ext) {
        if (dir && dir[0]) snprintf(p, n, "%s/%s", dir, spec);
        else               snprintf(p, n, "%s", spec);
        return path_normalize(p);
    }
    
    const char *slash = strrchr(spec, '/');
    const char *last  = slash ? slash + 1 : spec;
    if (root[0]) snprintf(p, n, "%s/std/%s/%s.salam", root, spec, last);
    else         snprintf(p, n, "std/%s/%s.salam", spec, last);
    
    if (!file_exists(p)) {
        const char *aliased = resolve_pkg_alias(a, root, spec);
        if (aliased) return aliased;
    }
    return p;
}

static const char *import_local_name(arena_t *a, ast_node_t *imp, const char *spec)
{
    if (imp->name) return imp->name;                 
    const char *slash = strrchr(spec, '/');
    const char *seg = slash ? slash + 1 : spec;
    const char *dot = strstr(seg, ".salam");
    size_t len = dot ? (size_t)(dot - seg) : strlen(seg);
    return arena_strndup(a, seg, len);
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

static const char *base_of(const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *bs = strrchr(path, '\\');
    const char *cut = slash;
    if (bs && (!slash || bs > slash)) cut = bs;
    return cut ? cut + 1 : path;
}

int salam_package_files(arena_t *a, const char *main_path, const char **out, int max)
{
    if (max <= 0) return 0;
    int n = 0;
    out[n++] = main_path;
    const char *dir = dir_of(a, main_path);
    const char *mainbase = base_of(main_path);

    {
        const char *dot = strrchr(mainbase, '.');
        size_t stemlen = dot ? (size_t)(dot - mainbase) : strlen(mainbase);
        const char *dirseg = (dir && dir[0]) ? base_of(dir) : "";
        if (strlen(dirseg) != stemlen || strncmp(dirseg, mainbase, stemlen) != 0)
            return n;   /* not a package main file: load it alone */
    }

    char dbuf[1024];
    if (!dir || !dir[0]) snprintf(dbuf, sizeof dbuf, ".");
    else                 snprintf(dbuf, sizeof dbuf, "%s", dir);
#if defined(_WIN32)
    char pat[1100];
    snprintf(pat, sizeof pat, "%s/*.salam", dbuf);
    struct _finddata_t fd;
    intptr_t h = _findfirst(pat, &fd);
    if (h != -1) {
        do {
            if (fd.attrib & _A_SUBDIR) continue;
            if (strcmp(fd.name, mainbase) == 0) continue;
            if (n >= max) break;
            char *full = (char *)arena_alloc(a, strlen(dbuf) + strlen(fd.name) + 2);
            snprintf(full, strlen(dbuf) + strlen(fd.name) + 2, "%s/%s", dbuf, fd.name);
            out[n++] = full;
        } while (_findnext(h, &fd) == 0);
        _findclose(h);
    }
#else
    DIR *d = opendir(dbuf);
    if (d) {
        struct dirent *e;
        while ((e = readdir(d)) != NULL && n < max) {
            size_t L = strlen(e->d_name);
            if (L <= 6 || strcmp(e->d_name + L - 6, ".salam") != 0) continue;
            if (strcmp(e->d_name, mainbase) == 0) continue;
            char *full = (char *)arena_alloc(a, strlen(dbuf) + L + 2);
            snprintf(full, strlen(dbuf) + L + 2, "%s/%s", dbuf, e->d_name);
            out[n++] = full;
        }
        closedir(d);
    }
#endif
    /* Stable order for reproducible builds; leave out[0] (main) pinned. */
    { int i = 2; for (; i < n; i++) {
        const char *key = out[i]; int j = i - 1;
        while (j >= 1 && strcmp(out[j], key) > 0) { out[j + 1] = out[j]; j--; }
        out[j + 1] = key;
    } }
    return n;
}

static const char *import_spec_of(ast_node_t *d)
{
    if (d->value.kind == TV_STRING && d->value.as.s) return d->value.as.s;
    return d->name;
}

void salam_merge_program(arena_t *a, ast_node_t *dst, ast_node_t *src)
{
    size_t i = 0;
    for (; i < src->list.len; i++) {
        ast_node_t *d = (ast_node_t *)src->list.data[i];
        if (d->kind == AST_IMPORT) {
            const char *spec = import_spec_of(d);
            bool dup = false;
            { size_t j = 0; for (; j < dst->list.len; j++) {
                ast_node_t *e = (ast_node_t *)dst->list.data[j];
                if (e->kind == AST_IMPORT) {
                    const char *es = import_spec_of(e);
                    if (spec && es && strcmp(spec, es) == 0) { dup = true; break; }
                }
            } }
            if (dup) continue;
        } else if (d->kind == AST_LINK) {
            const char *lib = (d->value.kind == TV_STRING && d->value.as.s) ? d->value.as.s : NULL;
            bool dup = false;
            if (lib) { size_t j = 0; for (; j < dst->list.len; j++) {
                ast_node_t *e = (ast_node_t *)dst->list.data[j];
                if (e->kind == AST_LINK) {
                    const char *el = (e->value.kind == TV_STRING && e->value.as.s) ? e->value.as.s : NULL;
                    if (el && strcmp(lib, el) == 0) { dup = true; break; }
                }
            } }
            if (dup) continue;
        }
        ast_add(a, dst, d);
    }
}

static void load_imports(sema_t *s, ast_node_t *program);
static symbol_t *pkg_cache_get(sema_t *s, const char *path)
{
    { size_t i = 0; for (; i + 1 < s->pkg_cache.len; i += 2)
        if (strcmp((const char *)s->pkg_cache.data[i], path) == 0)
            return (symbol_t *)s->pkg_cache.data[i + 1]; }
    return NULL;
}

static symbol_t *load_package(sema_t *s, const char *path, ast_node_t *imp)
{
    symbol_t *cached = pkg_cache_get(s, path);
    if (cached) return cached;
    { size_t i = 0; for (; i < s->loading.len; i++)
        if (strcmp((const char *)s->loading.data[i], path) == 0) {
            SERR(s, 8, &imp->span, "circular import detected: '%s'", path);
            return NULL;
        } }
    source_file_t *src = source_load(s->a, path);
    if (!src) { SERR(s, 8, &imp->span, "import not found: '%s'", path); return NULL; }
    src = preproc_source(s->a, s->log, src, NULL, 0);

    langpack_t *pack = langpack_load(i18n_lang());
    if (!pack) pack = langpack_load("en");
    token_stream_t *toks = NULL; lexer_run(s->a, s->log, pack, src, &toks);
    ast_node_t *prog = NULL;     parser_run(s->a, s->log, toks, &prog);

    { const char *files[256]; int nf = salam_package_files(s->a, path, files, 256);
      int fi = 1; for (; fi < nf; fi++) {
        source_file_t *fsrc = source_load(s->a, files[fi]);
        if (!fsrc) continue;
        fsrc = preproc_source(s->a, s->log, fsrc, NULL, 0);
        token_stream_t *ftoks = NULL; lexer_run(s->a, s->log, pack, fsrc, &ftoks);
        ast_node_t *fprog = NULL;     parser_run(s->a, s->log, ftoks, &fprog);
        salam_merge_program(s->a, prog, fprog);
    } }
    LOG_D(s->log, PH_SEMANTIC, "resolving package %s", path);
    
    scope_t *pkgscope = scope_new(s->a, SCOPE_GLOBAL, NULL);
    pkgscope->label = prog->name;
    scope_t *save_global = s->global, *save_cur = s->cur;
    const char *save_dir = s->dir, *save_pkg = s->pkg;
    ast_node_t *save_prog = s->program;
    vec_t save_pending = s->pending;
    s->global = pkgscope; s->cur = pkgscope;
    s->dir = dir_of(s->a, path);
    s->pkg = prog->name ? prog->name : "main";
    s->program = prog;                       
    vec_init(&s->pending);
    vec_push(s->a, &s->loading, CONST_CAST(path));
    load_imports(s, prog);       
    sema_collect(s, prog);
    sema_check_pass(s, prog);
    s->loading.len--;            
    s->global = save_global; s->cur = save_cur; s->dir = save_dir; s->pkg = save_pkg;
    s->program = save_prog; s->pending = save_pending;
    symbol_t *pk = symbol_new(s->a, SYM_PACKAGE, prog->name);
    pk->members = pkgscope;
    pk->pkgname = prog->name ? prog->name : "main";
    pk->decl = prog;
    vec_push(s->a, &s->pkg_cache, CONST_CAST(path));
    vec_push(s->a, &s->pkg_cache, (void *)pk);
    return pk;
}

static void load_import_file(sema_t *s, ast_node_t *imp)
{
    const char *spec = (imp->value.kind == TV_STRING && imp->value.as.s) ? imp->value.as.s : imp->name;
    if (!spec) return;
    
    const char *path = salam_resolve_import(s->a, s->dir, spec);
    symbol_t *pk = load_package(s, path, imp);
    if (!pk) return;
    
    const char *local = import_local_name(s->a, imp, spec);
    symbol_t *bind = symbol_new(s->a, SYM_PACKAGE, local);
    bind->members = pk->members;
    bind->pkgname = pk->pkgname;
    bind->decl = pk->decl;
    if (scope_lookup_local(s->cur, local))
        SERR(s, 1, &imp->span, "duplicate import name '%s'", local);
    else
        scope_define(s->a, s->cur, bind);
    
    const char *alias = (pk->decl) ? alias_for_lang(&pk->decl->aliases, i18n_lang()) : NULL;
    if (alias && strcmp(alias, local) != 0 && !scope_lookup_local(s->cur, alias)) {
        symbol_t *b2 = symbol_new(s->a, SYM_PACKAGE, alias);
        b2->members = pk->members;
        b2->pkgname = pk->pkgname;
        b2->decl = pk->decl;
        scope_define(s->a, s->cur, b2);
    }
}

static void load_imports(sema_t *s, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_IMPORT) load_import_file(s, d);
    } }
}

void sema_load_prelude(sema_t *s)
{
    if (s->prelude || s->prelude_tried) return;
    s->prelude_tried = true;
    if (s->pkg && strcmp(s->pkg, "collections") == 0) return;
    const char *path = salam_resolve_import(s->a, "", "collections");
    if (!path) return;
    ast_node_t imp; memset(&imp, 0, sizeof(imp));
    symbol_t *pk = load_package(s, path, &imp);
    if (pk) s->prelude = pk->members;
}

sema_result_t *sema_run(arena_t *a, logger_t *log, ast_node_t *program, const char *file)
{
    sema_t s;
    memset(&s, 0, sizeof(s));
    s.a = a;
    s.log = log;
    s.file = file;
    s.tc = type_ctx_new(a);
    s.diag = diag_new(a, log, PH_SEMANTIC);
    s.global = scope_new(a, SCOPE_GLOBAL, NULL);
    s.cur = s.global;
    s.dir = dir_of(a, file);
    s.pkg = program->name ? program->name : "main";
    s.program = program;
    vec_init(&s.imported);
    vec_init(&s.pkg_cache);
    vec_init(&s.loading);
    vec_init(&s.pending);
    LOG_I(log, PH_SEMANTIC, "analyzing %zu top-level definitions", program->list.len);
    load_imports(&s, program);
    sema_collect(&s, program);
    sema_check_pass(&s, program);
    LOG_I(log, PH_SEMANTIC, "analysis complete: %zu error(s), %zu warning(s)",
          s.diag->errors, s.diag->warnings);
    sema_result_t *r = (sema_result_t *)arena_alloc(a, sizeof(*r));
    r->global = s.global;
    r->tc = s.tc;
    r->errors = s.diag->errors;
    r->warnings = s.diag->warnings;
    r->ok = (s.diag->errors == 0);
    
    vec_init(&r->packages);
    { size_t i = 1; for (; i < s.pkg_cache.len; i += 2)
        vec_push(a, &r->packages, s.pkg_cache.data[i]); }
    return r;
}

static const char *sym_kind_name(sym_kind_t k)
{
    switch (k) {
        case SYM_VAR: return "var"; case SYM_CONST: return "const";
        case SYM_PARAM: return "param"; case SYM_FIELD: return "field";
        case SYM_FUNC: return "function"; case SYM_METHOD: return "method";
        case SYM_STRUCT: return "struct"; case SYM_ENUM: return "enum";
        case SYM_ALIAS: return "alias"; case SYM_ENUM_MEMBER: return "enum_member";
        case SYM_PACKAGE: return "package"; case SYM_INTERFACE: return "interface";
        case SYM_TYPEIMPL: return "typeimpl";
    }
    return "?";
}

static void emit_sig(xml_writer_t *w, type_ctx_t *tc, func_sig_t *sig)
{
    xml_open(w, "overload");
    xml_attr(w, "mangled", sig->mangled ? sig->mangled : "");
    xml_attr(w, "returns", type_to_string(tc, sig->ret));
    xml_open(w, "params");
    { size_t i = 0; for (; i < sig->params.len; i++)
        xml_leaf(w, "param", type_to_string(tc, (type_t *)sig->params.data[i])); }
    xml_close(w);
    xml_close(w);
}

static void emit_symbol(xml_writer_t *w, type_ctx_t *tc, symbol_t *sym)
{
    xml_open(w, "symbol");
    xml_attr(w, "kind", sym_kind_name(sym->kind));
    xml_attr(w, "name", sym->name);
    if (sym->type && sym->kind != SYM_STRUCT && sym->kind != SYM_ENUM &&
        sym->kind != SYM_FUNC && sym->kind != SYM_METHOD)
        xml_attr(w, "type", type_to_string(tc, sym->type));
    if (sym->kind == SYM_VAR || sym->kind == SYM_CONST)
        xml_attr(w, "mut", sym->is_mut ? "true" : "false");
    if (sym->kind == SYM_ENUM_MEMBER) {
        char buf[32]; sal_i64toa((int64_t)sym->enum_value, buf);
        xml_attr(w, "value", buf);
    }
    if (sym->kind == SYM_FUNC || sym->kind == SYM_METHOD) {
        { size_t i = 0; for (; i < sym->overloads.len; i++)
            emit_sig(w, tc, (func_sig_t *)sym->overloads.data[i]); }
    }
    if (sym->members) {
        xml_open(w, "members");
        { size_t i = 0; for (; i < sym->members->symbols.len; i++)
            emit_symbol(w, tc, (symbol_t *)sym->members->symbols.data[i]); }
        xml_close(w);
    }
    xml_close(w);
}

void symbols_to_xml(xml_writer_t *w, const sema_result_t *r)
{
    xml_open(w, "symbols");
    xml_attr_int(w, "errors", (long long)r->errors);
    xml_attr_int(w, "warnings", (long long)r->warnings);
    { size_t i = 0; for (; i < r->global->symbols.len; i++)
        emit_symbol(w, r->tc, (symbol_t *)r->global->symbols.data[i]); }
    xml_close(w);
}
