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
#else
#  include <dirent.h>
#endif

static const char *g_stdlib_root = NULL;
void salam_set_stdlib_root(const char *root) { g_stdlib_root = root; }

static char *path_normalize(char *p)
{
    
    while (p[0] == '.' && p[1] == '/') memmove(p, p + 2, strlen(p + 2) + 1);
    
    char *q;
    while ((q = strstr(p, "/./")) != NULL) memmove(q + 1, q + 3, strlen(q + 3) + 1);
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
    const char *root = g_stdlib_root ? g_stdlib_root : "";
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
