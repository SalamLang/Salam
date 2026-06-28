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

#include "codegen/codegen_internal.h"
#include "core/sal_format.h"
#include "semantic/sema.h"   

static void vec_collect_one(cg_t *cg, const char *ts)
{
    if (!ts || strncmp(ts, "Vector<", 7) != 0) return;
    { size_t i = 0; for (; i < cg->vec_types.len; i++)
        if (!strcmp((const char *)cg->vec_types.data[i], ts)) return; }
    vec_push(cg->a, &cg->vec_types, (void *)arena_strdup(cg->a, ts));
}

static void vec_collect(cg_t *cg, ast_node_t *n)
{
    if (!n) return;
    vec_collect_one(cg, n->type_str);
    vec_collect(cg, n->type); vec_collect(cg, n->a); vec_collect(cg, n->b);
    vec_collect(cg, n->c); vec_collect(cg, n->d);
    { size_t i = 0; for (; i < n->list.len; i++) vec_collect(cg, (ast_node_t *)n->list.data[i]); }
    { size_t i = 0; for (; i < n->dims.len; i++) vec_collect(cg, (ast_node_t *)n->dims.data[i]); }
}

static void vec_collect_types(cg_t *cg, ast_node_t *program)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_STRUCT_DEF) {
            symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
            if (!ssym) continue;
            { size_t j = 0; for (; j < ssym->members->symbols.len; j++) {
                symbol_t *f = (symbol_t *)ssym->members->symbols.data[j];
                if (f->kind == SYM_FIELD && f->type)
                    vec_collect_one(cg, type_to_string(cg->sem->tc, f->type));
            } }
        }
    } }
}

static void dyn_set_add(cg_t *cg, vec_t *set, const char *s)
{
    { size_t i = 0; for (; i < set->len; i++)
        if (!strcmp((const char *)set->data[i], s)) return; }
    vec_push(cg->a, set, (void *)arena_strdup(cg->a, s));
}

static void dyn_collect(cg_t *cg, ast_node_t *n)
{
    if (!n) return;
    if (n->type_str && !strncmp(n->type_str, "dyn ", 4))
        dyn_set_add(cg, &cg->dyn_ifaces, n->type_str + 4);
    if (n->kind == AST_TYPE && n->is_dyn && n->name)
        dyn_set_add(cg, &cg->dyn_ifaces, n->name);
    if (n->kind == AST_CAST && n->type_str && !strncmp(n->type_str, "dyn ", 4) &&
        n->a && n->a->type_str) {
        const char *iface = n->type_str + 4;
        dyn_set_add(cg, &cg->dyn_ifaces, iface);
        char pair[256]; sal_snprintf(pair, sizeof pair, "%s|%s", iface, n->a->type_str);
        dyn_set_add(cg, &cg->dyn_impls, pair);
    }
    dyn_collect(cg, n->type); dyn_collect(cg, n->a); dyn_collect(cg, n->b);
    dyn_collect(cg, n->c); dyn_collect(cg, n->d);
    { size_t i = 0; for (; i < n->list.len; i++) dyn_collect(cg, (ast_node_t *)n->list.data[i]); }
    { size_t i = 0; for (; i < n->dims.len; i++) dyn_collect(cg, (ast_node_t *)n->dims.data[i]); }
}

static func_sig_t *dyn_match_sig(symbol_t *cm, func_sig_t *want)
{
    if (!cm) return NULL;
    { size_t k = 0; for (; k < cm->overloads.len; k++) {
        func_sig_t *h = (func_sig_t *)cm->overloads.data[k];
        if (h->params.len != want->params.len) continue;
        bool ok = true;
        { size_t p = 0; for (; p < want->params.len; p++)
            if (!type_equiv((type_t *)h->params.data[p], (type_t *)want->params.data[p])) { ok = false; break; } }
        if (ok) return h;
    } }
    return cm->overloads.len ? (func_sig_t *)cm->overloads.data[0] : NULL;
}

static void cg_emit_dyn_types(cg_t *cg, sb_t *h)
{
    { size_t i = 0; for (; i < cg->dyn_ifaces.len; i++) {
        const char *I = (const char *)cg->dyn_ifaces.data[i];
        const char *cI = cg_cident(cg, I);
        symbol_t *isym = scope_lookup(cg->sem->global, I);
        if (!isym || isym->kind != SYM_INTERFACE) continue;
        sb_puts(h, cg_fmt(cg, "#ifndef SALAM_DYN_%s_DEFINED\n#define SALAM_DYN_%s_DEFINED\n", cI, cI));
        sb_puts(h, cg_fmt(cg, "typedef struct _Salam_vtbl_%s {\n", cI));
        { size_t m = 0; for (; m < isym->members->symbols.len; m++) {
            symbol_t *ms = (symbol_t *)isym->members->symbols.data[m];
            if (ms->kind != SYM_METHOD || !ms->overloads.len) continue;
            func_sig_t *sig = (func_sig_t *)ms->overloads.data[0];
            sb_puts(h, cg_fmt(cg, "    %s (*%s)(void*",
                    cg_ctype(cg, type_to_string(cg->sem->tc, sig->ret)), cg_cident(cg, ms->name)));
            { size_t p = 0; for (; p < sig->params.len; p++)
                sb_puts(h, cg_fmt(cg, ", %s", cg_ctype(cg, type_to_string(cg->sem->tc, (type_t *)sig->params.data[p])))); }
            sb_puts(h, ");\n");
        } }
        sb_puts(h, cg_fmt(cg, "} _Salam_vtbl_%s;\n", cI));
        sb_puts(h, cg_fmt(cg, "typedef struct { void* data; const _Salam_vtbl_%s* vtable; } _Salam_dyn_%s;\n", cI, cI));
        sb_puts(h, "#endif\n");
    } }
}

void cg_emit_dyn_vtables(cg_t *cg, sb_t *c)
{
    { size_t i = 0; for (; i < cg->dyn_impls.len; i++) {
        char buf[256]; sal_snprintf(buf, sizeof buf, "%s", (const char *)cg->dyn_impls.data[i]);
        char *bar = strchr(buf, '|'); if (!bar) continue; *bar = 0;
        const char *I = buf, *C = bar + 1;
        const char *cI = cg_cident(cg, I), *cC = cg_cident(cg, C);
        symbol_t *isym = scope_lookup(cg->sem->global, I);
        symbol_t *csym = scope_lookup(cg->sem->global, C);
        if (!isym || isym->kind != SYM_INTERFACE || !csym || csym->kind != SYM_STRUCT) continue;
        { size_t m = 0; for (; m < isym->members->symbols.len; m++) {
            symbol_t *ms = (symbol_t *)isym->members->symbols.data[m];
            if (ms->kind != SYM_METHOD || !ms->overloads.len) continue;
            func_sig_t *isig = (func_sig_t *)ms->overloads.data[0];
            symbol_t *cm = scope_lookup_local(csym->members, ms->name);
            func_sig_t *csig = dyn_match_sig(cm, isig);
            if (!csig) continue;
            const char *mangled = cg_mangle(cg, csym->name, ms->name, &csig->params);
            const char *ret = cg_ctype(cg, type_to_string(cg->sem->tc, isig->ret));
            sb_t ps; sb_init(&ps); sb_t as; sb_init(&as);
            { size_t p = 0; for (; p < isig->params.len; p++) {
                const char *pt = cg_ctype(cg, type_to_string(cg->sem->tc, (type_t *)isig->params.data[p]));
                sb_puts(&ps, cg_fmt(cg, ", %s __a%zu", pt, p));
                sb_puts(&as, cg_fmt(cg, ", __a%zu", p));
            } }
            bool isvoid = isig->ret && isig->ret->kind == TY_VOID;
            sb_puts(c, cg_fmt(cg, "static %s _Salam_thunk_%s_%s_%s(void* self%s){ %s%s((%s*)self%s); }\n",
                    ret, cI, cC, cg_cident(cg, ms->name), sb_cstr(&ps),
                    isvoid ? "" : "return ", mangled, cC, sb_cstr(&as)));
            sb_free(&ps); sb_free(&as);
        } }
        sb_puts(c, cg_fmt(cg, "static const _Salam_vtbl_%s _Salam_vtbl_%s_%s = {", cI, cI, cC));
        bool first = true;
        { size_t m = 0; for (; m < isym->members->symbols.len; m++) {
            symbol_t *ms = (symbol_t *)isym->members->symbols.data[m];
            if (ms->kind != SYM_METHOD || !ms->overloads.len) continue;
            sb_puts(c, cg_fmt(cg, "%s _Salam_thunk_%s_%s_%s", first ? "" : ",", cI, cC, cg_cident(cg, ms->name)));
            first = false;
        } }
        sb_puts(c, " };\n");
    } }
    if (cg->dyn_impls.len) sb_putc(c, '\n');
}

static void vec_emit_typedef(cg_t *cg, sb_t *h, const char *ts)
{
    char elem[96]; cg_vec_elem(ts, elem, sizeof(elem));
    const char *E  = cg_ctype(cg, elem);        
    const char *cn = cg_vec_cname(cg, ts);      
    sb_puts(h, cg_fmt(cg, "#ifndef SALAM_%s_TYPEDEF\n#define SALAM_%s_TYPEDEF\n", cn, cn));
    sb_puts(h, cg_fmt(cg, "typedef struct { %s* data; int32_t len; int32_t cap; } %s;\n", E, cn));
    sb_puts(h, "#endif\n");
}

static void vec_emit(cg_t *cg, sb_t *h, const char *ts)
{
    char elem[96]; cg_vec_elem(ts, elem, sizeof(elem));
    const char *E  = cg_ctype(cg, elem);        
    const char *cn = cg_vec_cname(cg, ts);      
    
    sb_puts(h, cg_fmt(cg, "#ifndef SALAM_%s_DEFINED\n#define SALAM_%s_DEFINED\n", cn, cn));
    sb_puts(h, cg_fmt(cg,
        "static void %s_grow(%s* v, int32_t need){ if(need<=v->cap) return; int32_t c=v->cap?v->cap:4; while(c<need) c*=2; "
        "v->data=(%s*)salam_realloc(v->data,(unsigned long long)c*sizeof(%s)); v->cap=c; }\n", cn, cn, E, E));
    sb_puts(h, cg_fmt(cg,
        "static void %s_push(%s* v, %s x){ %s_grow(v, v->len+1); v->data[v->len++]=x; }\n", cn, cn, E, cn));
    sb_puts(h, cg_fmt(cg,
        "static %s %s_pop(%s* v){ return v->data[--v->len]; }\n", E, cn, cn));
    sb_puts(h, cg_fmt(cg,
        "static %s* %s_get(%s* v, int32_t i){ if(i<0||i>=v->len) return (%s*)0; return &v->data[i]; }\n", E, cn, cn, E));
    sb_puts(h, cg_fmt(cg,
        "static void %s_set(%s* v, int32_t i, %s x){ if(i<0) return; %s_grow(v, i+1); if(i>=v->len) v->len=i+1; v->data[i]=x; }\n", cn, cn, E, cn));
    sb_puts(h, cg_fmt(cg, "static int32_t %s_len(%s* v){ return v->len; }\n", cn, cn));
    sb_puts(h, cg_fmt(cg, "static int32_t %s_cap(%s* v){ return v->cap; }\n", cn, cn));
    sb_puts(h, cg_fmt(cg, "static void %s_free(%s* v){ salam_free(v->data); v->data=(%s*)0; v->len=0; v->cap=0; }\n", cn, cn, E));
    sb_puts(h, "#endif\n");
}

static void hdr_prelude(cg_t *cg, ast_node_t *program, sb_t *h)
{
    sb_puts(h, "#pragma once\n");
    sb_puts(h, cg_fmt(cg, "#ifndef SALAM_MODULE_%s_H\n#define SALAM_MODULE_%s_H\n", cg->module, cg->module));
    
    sb_puts(h, "#include <stdint.h>\n#include <stdbool.h>\n#include <math.h>\n#include <stddef.h>\n");
    
    sb_puts(h, "#ifndef SALAM_RT_TYPES_DEFINED\n#define SALAM_RT_TYPES_DEFINED\n"
               "typedef struct salam_file salam_file;\n"
               "typedef struct salam_map salam_map;\n"
               "typedef struct salam_map_iter salam_map_iter;\n"
               "typedef struct { void* data; int64_t len; } salam_slice;\n"
               "extern int64_t salam_idx(int64_t, int64_t);\n"
               "static inline salam_slice salam_slice_new(void* b, int64_t lo, int64_t hi, int64_t esz)"
               "{ salam_slice s; s.data=(void*)((char*)b+(lo)*(esz)); s.len=(hi)-(lo); return s; }\n"
               "static inline salam_slice salam_slice_sub(salam_slice b, int64_t lo, int64_t hi, int has_hi, int64_t esz)"
               "{ salam_slice s; if(!has_hi) hi=b.len; s.data=(void*)((char*)b.data+(lo)*(esz)); s.len=(hi)-(lo); return s; }\n"
               "static inline void* salam_slice_at(salam_slice s, int64_t i, int64_t esz, int sf)"
               "{ return (void*)((char*)s.data+(sf?salam_idx(i,s.len):i)*(esz)); }\n"
               "typedef void (*salam_thread_fn)(void);\n"
               
               "extern void* salam_alloc(uint64_t size);\n"
               "extern void* salam_realloc(void* ptr, uint64_t size);\n"
               "extern void salam_free(void* ptr);\n#endif\n");
    if (strcmp(cg->module, "core") != 0)
        sb_puts(h, cg_fmt(cg, "#include \"%score.h\"\n", SALAM_MOD_PREFIX));
    
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_IMPORT) {
            const char *p = (d->value.kind == TV_STRING && d->value.as.s) ? d->value.as.s : d->name;
            if (!p) continue;
            const char *resolved = salam_resolve_import(cg->a, "", p);
            const char *slash = strrchr(resolved, '/');
            const char *bs    = strrchr(resolved, '\\');
            const char *stem  = slash ? slash + 1 : resolved;
            if (bs && (!slash || bs > slash)) stem = bs + 1;
            char base[128]; size_t k = 0;
            { const char *q = stem; for (; *q && *q != '.' && k < sizeof(base)-1; q++) base[k++] = *q; }
            base[k] = 0;
            sb_puts(h, cg_fmt(cg, "#include \"%s%s.h\"\n", SALAM_MOD_PREFIX, base));
        }
    } }
    sb_puts(h, "\n");
}

static void hdr_enums(cg_t *cg, ast_node_t *program, sb_t *h)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_ENUM_DEF) continue;
        sb_puts(h, "typedef enum {");
        long long next = 0;
        { size_t j = 0; for (; j < d->list.len; j++) {
            ast_node_t *m = (ast_node_t *)d->list.data[j];
            if (m->a && m->a->kind == AST_LITERAL && m->a->value.kind == TV_INT)
                next = (long long)m->a->value.as.i;
            sb_puts(h, cg_fmt(cg, "%s %s_%s = %lld", j ? "," : "",
                    cg_cident(cg, d->name), cg_cident(cg, m->name), next));
            next++;
        } }
        sb_puts(h, cg_fmt(cg, " } %s;\n", cg_cident(cg, d->name)));
    } }
}

static void hdr_struct_fwd(cg_t *cg, ast_node_t *program, sb_t *h)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind == AST_STRUCT_DEF) {
            if (d->typarams.len > 0) continue;   
            symbol_t *ssym2 = scope_lookup_local(cg->sem->global, d->name);
            const char *raw2 = (ssym2 && ssym2->type && ssym2->type->name) ? ssym2->type->name : d->name;
            const char *cn = cg_cident(cg, raw2);
            
            sb_puts(h, cg_fmt(cg, "#ifndef SALAM_FWD_%s\n#define SALAM_FWD_%s\n"
                                  "typedef struct %s %s;\n#endif\n", cn, cn, cn, cn));
        }
    } }
}

static void emit_struct_def(cg_t *cg, symbol_t *ssym, const char *gcn, sb_t *h)
{
    sb_puts(h, cg_fmt(cg, "#ifndef SALAM_DEF_%s\n#define SALAM_DEF_%s\n", gcn, gcn));
    sb_puts(h, cg_fmt(cg, "struct %s {\n", gcn));
    int nfields = 0;
    { size_t j = 0; for (; j < ssym->members->symbols.len; j++) {
        symbol_t *f = (symbol_t *)ssym->members->symbols.data[j];
        if (f->kind != SYM_FIELD) continue;
        sb_puts(h, cg_fmt(cg, "    %s;\n",
                cg_decl(cg, type_to_string(cg->sem->tc, f->type), f->name)));
        nfields++;
    } }

    if (nfields == 0)
        sb_puts(h, "    char _salam_empty;\n");
    sb_puts(h, "};\n#endif\n");
}

static const char *struct_dep_cname(cg_t *cg, type_t *t)
{
    while (t && t->kind == TY_ARRAY) t = t->elem;   
    if (t && (t->kind == TY_STRUCT || t->kind == TY_VEC || t->kind == TY_MAP_ITER))
        return cg_ctype(cg, type_to_string(cg->sem->tc, t));
    return NULL;
}

static void hdr_struct_visit(cg_t *cg, sb_t *h, size_t i, symbol_t **syms,
                             const char **gcns, size_t n, char *state)
{
    if (state[i]) return;
    state[i] = 1;
    symbol_t *ssym = syms[i];
    { size_t j = 0; for (; j < ssym->members->symbols.len; j++) {
        symbol_t *f = (symbol_t *)ssym->members->symbols.data[j];
        if (f->kind != SYM_FIELD) continue;
        const char *dep = struct_dep_cname(cg, f->type);
        if (!dep) continue;
        { size_t k = 0; for (; k < n; k++)
            if (k != i && !strcmp(dep, gcns[k]))
                hdr_struct_visit(cg, h, k, syms, gcns, n, state); }
    } }
    emit_struct_def(cg, ssym, gcns[i], h);
    state[i] = 2;
}

static void hdr_structs(cg_t *cg, ast_node_t *program, sb_t *h)
{
    size_t cap = program->list.len;
    symbol_t  **syms = arena_alloc(cg->a, cap * sizeof(*syms));
    const char **gcns = arena_alloc(cg->a, cap * sizeof(*gcns));
    char *state = arena_alloc(cg->a, cap ? cap : 1);
    memset(state, 0, cap);
    size_t n = 0;
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_STRUCT_DEF) continue;
        if (d->typarams.len > 0) continue;
        symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
        if (!ssym || !ssym->members) continue;
        const char *raw_cn = (ssym->type && ssym->type->name) ? ssym->type->name : d->name;
        syms[n] = ssym;
        gcns[n] = cg_cident(cg, raw_cn);
        n++;
    } }
    
    { size_t i = 0; for (; i < n; i++)
        hdr_struct_visit(cg, h, i, syms, gcns, n, state); }
}

static void hdr_aliases(cg_t *cg, ast_node_t *program, sb_t *h)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->kind != AST_TYPE_ALIAS) continue;
        symbol_t *as = scope_lookup_local(cg->sem->global, d->name);
        if (!as) continue;                       
        sb_puts(h, cg_fmt(cg, "typedef %s;\n", cg_decl(cg, type_to_string(cg->sem->tc, as->type), d->name)));
    } }
}

static void hdr_externs(cg_t *cg, ast_node_t *program, sb_t *h)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (!d->is_extern) continue;
        if (d->kind == AST_FUNC_DEF) {
            symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
            func_sig_t *sig = sig_of_decl(fsym, d);
            if (sig) sb_puts(h, cg_fmt(cg, "%s;\n", cg_extern_proto(cg, d, sig)));
        } else if (d->kind == AST_VAR_DECL) {
            const char *ts = d->type_str ? d->type_str : "int32_t";
            sb_puts(h, cg_fmt(cg, "extern %s;\n", cg_decl(cg, ts, d->name)));
        }
    } }
    sb_puts(h, "\n");
}

static void hdr_prototypes(cg_t *cg, ast_node_t *program, sb_t *h)
{
    { size_t i = 0; for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (d->typarams.len > 0) continue;
        if (d->is_extern) continue;
        if (d->synthetic) continue;   
        if (d->kind == AST_FUNC_DEF && strcmp(d->name, cg->entry) != 0 &&
            d->is_pub) {
            symbol_t *fsym = scope_lookup_local(cg->sem->global, d->name);
            func_sig_t *sig = sig_of_decl(fsym, d);
            if (sig) sb_puts(h, cg_fmt(cg, "%s;\n", func_signature(cg, d, NULL, sig, false)));
        } else if (d->kind == AST_STRUCT_DEF) {
            symbol_t *ssym = scope_lookup_local(cg->sem->global, d->name);
            if (!ssym || !ssym->members) continue;
            { size_t j = 0; for (; j < d->list.len; j++) {
                ast_node_t *m = (ast_node_t *)d->list.data[j];
                if (m->kind != AST_FUNC_DEF) continue;
                symbol_t *msym = scope_lookup_local(ssym->members, m->name);
                func_sig_t *sig = sig_of_decl(msym, m);
                if (sig) sb_puts(h, cg_fmt(cg, "%s;\n", func_signature(cg, m, ssym, sig, false)));
            } }
        }
    } }
    
}

void cg_header(cg_t *cg, ast_node_t *program)
{
    sb_t *h = cg->h;
    hdr_prelude(cg, program, h);
    hdr_enums(cg, program, h);
    hdr_struct_fwd(cg, program, h);
    
    dyn_collect(cg, program);
    cg_emit_dyn_types(cg, h);
    sb_puts(h, "\n");
    
    vec_collect(cg, program);
    vec_collect_types(cg, program);
    { size_t i = 0; for (; i < cg->vec_types.len; i++)
        vec_emit_typedef(cg, h, (const char *)cg->vec_types.data[i]); }
    sb_puts(h, "\n");
    hdr_structs(cg, program, h);
    hdr_aliases(cg, program, h);
    
    { size_t i = 0; for (; i < cg->vec_types.len; i++)
        vec_emit(cg, h, (const char *)cg->vec_types.data[i]); }
    sb_puts(h, "\n");
    hdr_externs(cg, program, h);
    hdr_prototypes(cg, program, h);
}
