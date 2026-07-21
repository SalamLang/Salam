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

#include "jsgen/jsgen_internal.h"
#include "core/sal_format.h"
#include "i18n/i18n.h"
#include "semantic/builtins.h"

static const char *jsg_lambda(jg_t *g, ast_node_t *n);

static bool jsg_reserved(const char *s)
{
    static const char *const kw[] = {
        "arguments", "await",      "break",     "case",      "catch",      "class",
        "const",     "continue",   "debugger",  "default",   "delete",     "do",
        "else",      "enum",       "eval",      "export",    "extends",    "false",
        "finally",   "for",        "function",  "if",        "implements", "import",
        "in",        "instanceof", "interface", "let",       "new",        "null",
        "of",        "package",    "private",   "protected", "public",     "return",
        "static",    "super",      "switch",    "this",      "throw",      "true",
        "try",       "typeof",     "undefined", "var",       "void",       "while",
        "with",      "yield",
    };
    static const char *const host[] = {
        "document",
        "window",
        "console",
        "Math",
        "JSON",
        "String",
        "Number",
        "Boolean",
        "Array",
        "Object",
        "Error",
        "Date",
        "Promise",
        "Symbol",
        "Map",
        "Set",
        "globalThis",
        "performance",
        "navigator",
        "location",
        "fetch",
        "alert",
        "confirm",
        "prompt",
        "parseInt",
        "parseFloat",
        "isNaN",
        "isFinite",
        "getComputedStyle",
        "requestAnimationFrame",
        "setTimeout",
        "setInterval",
        "clearTimeout",
        "clearInterval",
    };
    static const char *const runtime[] = {
        "salam_panic",
        "__salam_unsupported",
        "salam_input",
        "salam_strcat",
        "salam_str_hash",
        "salam_hash_int",
        "fmod",
        "sqrt",
        "cbrt",
        "sin",
        "cos",
        "tan",
        "asin",
        "acos",
        "atan",
        "atan2",
        "exp",
        "log",
        "log2",
        "log10",
        "floor",
        "ceil",
        "round",
        "trunc",
        "hypot",
        "fabs",
        "pow",
    };
    {
        size_t i = 0;
        for (; i < sizeof(kw) / sizeof(kw[0]); i++)
            if (strcmp(kw[i], s) == 0) return true;
    }
    {
        size_t i = 0;
        for (; i < sizeof(host) / sizeof(host[0]); i++)
            if (strcmp(host[i], s) == 0) return true;
    }
    {
        size_t i = 0;
        for (; i < sizeof(runtime) / sizeof(runtime[0]); i++)
            if (strcmp(runtime[i], s) == 0) return true;
    }
    return false;
}

const char *jsg_ident(jg_t *g, const char *name)
{
    bool plain = name && name[0] && !isdigit((unsigned char)name[0]);
    {
        const char *p = name;
        for (; plain && *p; p++)
            if (!(isalnum((unsigned char)*p) || *p == '_')) plain = false;
    }
    {
        const char *c = plain ? name : cg_cident(&g->cg, name);
        if (jsg_reserved(c)) return cg_fmt(&g->cg, "%s_", c);
        return c;
    }
}

static const char *jsg_minify_advance(jg_t *g)
{
    const char *prev = g->minify_last;
    char buf[256];
    if (!prev) {
        g->minify_last = arena_strdup(g->cg.a, "a");
        return g->minify_last;
    }
    {
        size_t len = strlen(prev);
        long i = (long)len - 1;
        memcpy(buf, prev, len + 1);
        for (; i >= 0; i--) {
            if (buf[i] != 'z') {
                buf[i]++;
                break;
            }
            buf[i] = 'a';
        }
        if (i < 0) {
            memmove(buf + 1, buf, len + 1);
            buf[0] = 'a';
        }
    }
    g->minify_last = arena_strdup(g->cg.a, buf);
    return g->minify_last;
}

const char *jsg_minify_next(jg_t *g)
{
    const char *cand;
    do {
        cand = jsg_minify_advance(g);
    } while (jsg_reserved(cand));
    return cand;
}

const char *jsg_minify_cached(jg_t *g, const char *key)
{
    size_t i = 0;
    for (; i < g->minify_keys.len; i++)
        if (!strcmp((const char *)g->minify_keys.data[i], key))
            return (const char *)g->minify_vals.data[i];
    {
        const char *name = jsg_minify_next(g);
        vec_push(g->cg.a, &g->minify_keys, CONST_CAST(key));
        vec_push(g->cg.a, &g->minify_vals, CONST_CAST(name));
        return name;
    }
}

static bool name_in_vec(vec_t *v, const char *name)
{
    size_t i = 0;
    for (; i < v->len; i++)
        if (strcmp((const char *)v->data[i], name) == 0) return true;
    return false;
}

bool jsg_taken(jg_t *g, const char *name)
{
    return name_in_vec(&g->taken, name);
}

static const char *jsg_fresh_syn(jg_t *g)
{
    static const char *const bases[] = {"i", "j", "k", "t", "u"};
    size_t b = 0;
    for (; b < sizeof(bases) / sizeof(bases[0]); b++) {
        const char *cand = bases[b];
        if (!local_known(&g->cg, cand) && !name_in_vec(&g->fn_used_names, cand) &&
            !jsg_taken(g, cand)) {
            vec_push(g->cg.a, &g->fn_used_names, CONST_CAST(cand));
            return cand;
        }
    }
    return jsg_fresh(g, "i");
}

const char *jsg_local(jg_t *g, const char *name, bool is_mut)
{
    const char *emitted;
    if (name && name[0] == '_' && name[1] == '_' && strcmp(name, "__self") != 0)
        emitted = jsg_fresh_syn(g);
    else if (g->enable_minify)
        emitted = jsg_minify_next(g);
    else
        emitted = jsg_ident(g, name);
    local_add(&g->cg, name);
    vec_push(g->cg.a, &g->local_muts, (void *)(is_mut ? (size_t)1 : (size_t)0));
    vec_push(g->cg.a, &g->local_emit, CONST_CAST(emitted));
    return emitted;
}

const char *jsg_local_ref(jg_t *g, const char *name)
{
    size_t i = g->cg.locals.len;
    for (; i > 0; i--)
        if (strcmp((const char *)g->cg.locals.data[i - 1], name) == 0)
            return (const char *)g->local_emit.data[i - 1];
    return jsg_ident(g, name);
}

void jsg_scope_reset(jg_t *g, size_t mark)
{
    g->cg.locals.len = mark;
    g->local_muts.len = mark;
    g->local_emit.len = mark;
}

bool jsg_local_is_mut(jg_t *g, const char *name)
{
    size_t i = g->cg.locals.len;
    for (; i > 0; i--)
        if (strcmp((const char *)g->cg.locals.data[i - 1], name) == 0)
            return g->local_muts.data[i - 1] != NULL;
    return true;
}

const char *jsg_fresh(jg_t *g, const char *base)
{
    const char *safe = jsg_ident(g, base);
    int i = 1;
    for (;; i++) {
        const char *cand = (i == 1) ? safe : cg_fmt(&g->cg, "%s%d", safe, i);
        if (!local_known(&g->cg, cand) && !name_in_vec(&g->fn_used_names, cand)) {
            vec_push(g->cg.a, &g->fn_used_names, CONST_CAST(cand));
            return cand;
        }
    }
}

const char *jsg_fn_name(jg_t *g, const char *pkg, const char *sname, const char *fname,
                        symbol_t *fsym, func_sig_t *sig, bool is_extern, bool synthetic)
{
    cg_t *cg = &g->cg;
    const char *p = pkg ? pkg : "main";
    if (is_extern) return cg_cident(cg, fname);
    if (synthetic && sig) return cg_mangle_in(cg, "g", sname, fname, &sig->params);
    if (fsym && fsym->overloads.len > 1 && sig)
        return cg_mangle_in(cg, p, sname, fname, &sig->params);
    if (!strcmp(p, "main") && !sname && jsg_taken(g, fname) && sig)
        return cg_mangle_in(cg, p, sname, fname, &sig->params);
    if (g->enable_minify)
        return jsg_minify_cached(g,
                                 cg_fmt(cg, "F:%s:%s:%s", p, sname ? sname : "", fname));
    {
        const char *base;
        if (sname)
            base = strcmp(p, "main") ? cg_fmt(cg, "%s_%s_%s", p, sname, fname)
                                     : cg_fmt(cg, "%s_%s", sname, fname);
        else
            base = strcmp(p, "main") ? cg_fmt(cg, "%s_%s", p, fname) : fname;
        return jsg_ident(g, base);
    }
}

const char *jsg_global_ref(jg_t *g, const char *pkg, const char *name)
{
    cg_t *cg = &g->cg;
    const char *p = pkg ? pkg : "main";
    if (g->enable_minify) return jsg_minify_cached(g, cg_fmt(cg, "G:%s:%s", p, name));
    if (!strcmp(p, "main")) {
        if (jsg_taken(g, name))
            return cg_fmt(cg, "_SalamG_%s_%s", cg_cident(cg, p), cg_cident(cg, name));
        return jsg_ident(g, name);
    }
    return jsg_ident(g, cg_fmt(cg, "%s_%s", p, name));
}

const char *jsg_escape(jg_t *g, const char *s)
{
    sb_t b;
    sb_init(&b);
    sb_putc(&b, '"');
    {
        const char *p = s;
        for (; *p; p++) {
            unsigned char ch = (unsigned char)*p;
            switch (ch) {
            case '\n':
                sb_puts(&b, "\\n");
                break;
            case '\t':
                sb_puts(&b, "\\t");
                break;
            case '\r':
                sb_puts(&b, "\\r");
                break;
            case '"':
                sb_puts(&b, "\\\"");
                break;
            case '\\':
                sb_puts(&b, "\\\\");
                break;
            default:
                if (ch < 0x20 || ch == 0x7f) {
                    char u[8];
                    sal_snprintf(u, sizeof(u), "\\u%04x", (unsigned)ch);
                    sb_puts(&b, u);
                } else {
                    sb_putc(&b, (char)ch);
                }
                break;
            }
        }
    }
    sb_putc(&b, '"');
    {
        const char *r = arena_strdup(g->cg.a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static void jsg_template_text(sb_t *b, const char *s)
{
    const char *p = s;
    for (; *p; p++) {
        unsigned char ch = (unsigned char)*p;
        if (ch == '`' || ch == '\\') {
            sb_putc(b, '\\');
            sb_putc(b, (char)ch);
        } else if (ch == '$' && p[1] == '{') {
            sb_puts(b, "\\$");
        } else if (ch == '\n') {
            sb_puts(b, "\\n");
        } else if (ch == '\t') {
            sb_puts(b, "\\t");
        } else if (ch == '\r') {
            sb_puts(b, "\\r");
        } else if (ch < 0x20 || ch == 0x7f) {
            char u[8];
            sal_snprintf(u, sizeof(u), "\\u%04x", (unsigned)ch);
            sb_puts(b, u);
        } else {
            sb_putc(b, (char)ch);
        }
    }
}

const char *jsg_zero(jg_t *g, const char *ts)
{
    cg_t *cg = &g->cg;
    if (!ts) return "0";
    if (!strcmp(ts, "str") || !strcmp(ts, "uchar")) return "\"\"";
    if (!strcmp(ts, "bool")) return "false";
    if (cg_is_int_typestr(ts) || !strcmp(ts, "f32") || !strcmp(ts, "f64") ||
        !strcmp(ts, "char"))
        return "0";
    if (strchr(ts, '[')) {
        long sz = array_size_of(ts);
        if (sz > 0) return cg_fmt(cg, "new Array(%ld).fill(0)", sz);
        return "[]";
    }
    if (!strncmp(ts, "func(", 5)) return "null";
    if (struct_by_name(cg, ts)) return "{}";
    return "null";
}

static bool jsg_is_str_lit(const ast_node_t *n)
{
    return n && n->kind == AST_LITERAL &&
           (n->op == TK_STRING || n->op == TK_TRIPLE_STRING || n->op == TK_RAW_STRING ||
            n->op == TK_UTF8_CHAR);
}

static void jsg_concat_flatten(jg_t *g, ast_node_t *n, vec_t *segs)
{
    if (n && n->kind == AST_BINARY && n->op == TK_PLUS && n->type_str &&
        !strcmp(n->type_str, "str")) {
        jsg_concat_flatten(g, n->a, segs);
        jsg_concat_flatten(g, n->b, segs);
        return;
    }
    vec_push(g->cg.a, segs, n);
}

static const char *jsg_template_concat(jg_t *g, ast_node_t *n)
{
    vec_t segs;
    size_t nlit = 0, nexpr = 0;
    vec_init(&segs);
    jsg_concat_flatten(g, n, &segs);
    {
        size_t i = 0;
        for (; i < segs.len; i++) {
            if (jsg_is_str_lit((ast_node_t *)segs.data[i]))
                nlit++;
            else
                nexpr++;
        }
    }
    if (segs.len < 2 || nlit == 0 || nexpr == 0) return NULL;
    {
        sb_t b;
        sb_init(&b);
        sb_putc(&b, '`');
        {
            size_t i = 0;
            for (; i < segs.len; i++) {
                ast_node_t *s = (ast_node_t *)segs.data[i];
                if (jsg_is_str_lit(s)) {
                    jsg_template_text(&b, s->value.as.s ? s->value.as.s : "");
                } else {
                    sb_puts(&b, "${");
                    sb_puts(&b, jsg_expr_p(g, s, 0));
                    sb_putc(&b, '}');
                }
            }
        }
        sb_putc(&b, '`');
        {
            const char *r = arena_strdup(g->cg.a, sb_cstr(&b));
            sb_free(&b);
            return r;
        }
    }
}

static int jsg_binprec(token_kind_t k)
{
    switch (k) {
    case TK_STAR:
    case TK_SLASH:
    case TK_PERCENT:
        return 13;
    case TK_PLUS:
    case TK_MINUS:
        return 12;
    case TK_LT:
    case TK_GT:
    case TK_LE:
    case TK_GE:
        return 10;
    case TK_EQ:
    case TK_NE:
        return 9;
    case TK_AND:
        return 5;
    case TK_OR:
        return 4;
    default:
        return 12;
    }
}

static const char *jsg_op(token_kind_t k)
{
    switch (k) {
    case TK_EQ:
        return "===";
    case TK_NE:
        return "!==";
    default:
        return cg_op(k);
    }
}

static const char *jsg_wrap(jg_t *g, const char *s, int own, int minprec)
{
    if (own < minprec) return cg_fmt(&g->cg, "(%s)", s);
    return s;
}

const char *jsg_emit_op_call(jg_t *g, ast_node_t *lhs, symbol_t *ssym, const char *sname,
                             const char *mname, ast_node_t *rhs)
{
    cg_t *cg = &g->cg;
    symbol_t *m = scope_lookup_local(ssym->members, mname);
    if (!m || m->kind != SYM_METHOD) return NULL;
    {
        func_sig_t *sig = pick_op_overload(cg, m, rhs ? 1 : 0);
        if (!sig) return NULL;
        {
            const char *spkg = ssym->pkgname ? ssym->pkgname : cg->pkg;
            const char *fn =
                jsg_fn_name(g, spkg, ssym->name, mname, m, sig, false, false);
            if (rhs)
                return cg_fmt(cg, "%s(%s, %s)", fn, jsg_expr_p(g, lhs, 0),
                              jsg_expr_p(g, rhs, 0));
            return cg_fmt(cg, "%s(%s)", fn, jsg_expr_p(g, lhs, 0));
        }
    }
}

static const char *jsg_unsupported(jg_t *g, const char *what)
{
    LOG_W(g->cg.log, PH_CODEGEN, "js backend: '%s' is not supported yet", what);
    return cg_fmt(&g->cg, "__salam_unsupported(\"%s\")", what);
}

const char *jsg_call_args(jg_t *g, ast_node_t *call, func_sig_t *sig)
{
    cg_t *cg = &g->cg;
    sb_t b;
    sb_init(&b);
    {
        size_t i = 0;
        for (; i < call->list.len; i++) {
            if (i) sb_puts(&b, ", ");
            sb_puts(&b, jsg_expr_p(g, (ast_node_t *)call->list.data[i], 0));
        }
    }
    if (sig && sig->decl) {
        size_t np = sig->decl->list.len;
        size_t emitted = call->list.len;
        {
            size_t i = call->list.len;
            for (; i < np; i++) {
                ast_node_t *param = (ast_node_t *)sig->decl->list.data[i];
                if (!param->a) continue;
                if (emitted) sb_puts(&b, ", ");
                sb_puts(&b, jsg_expr_p(g, param->a, 0));
                emitted++;
            }
        }
    }
    {
        const char *r = arena_strdup(cg->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static const char *jsg_handler_base(jg_t *g, const char *extname, ast_node_t **args,
                                    size_t nargs)
{
    cg_t *cg = &g->cg;
    const char *ev = NULL;
    {
        size_t i = 0;
        for (; i < nargs; i++)
            if (jsg_is_str_lit(args[i]) && args[i]->value.as.s &&
                args[i]->value.as.s[0]) {
                ev = args[i]->value.as.s;
                break;
            }
    }
    if (ev) {
        sb_t b;
        sb_init(&b);
        sb_puts(&b, "on");
        {
            bool up = true;
            const char *p = ev;
            for (; *p; p++) {
                unsigned char ch = (unsigned char)*p;
                if (!isalnum(ch)) {
                    up = true;
                    continue;
                }
                sb_putc(&b, (char)(up ? toupper(ch) : ch));
                up = false;
            }
        }
        {
            const char *r = arena_strdup(cg->a, sb_cstr(&b));
            sb_free(&b);
            return r;
        }
    }
    if (!strcmp(extname, "salam_js_raf")) return "onFrame";
    if (!strcmp(extname, "salam_js_set_timeout")) return "onTimeout";
    if (!strcmp(extname, "salam_js_set_interval")) return "onTick";
    if (!strcmp(extname, "salam_js_on_ready")) return "onReady";
    if (!strcmp(extname, "salam_js_on_resize")) return "onResize";
    return "handler";
}

static bool jsg_should_hoist(ast_node_t *n)
{
    return n && n->kind == AST_LAMBDA && n->a && n->a->kind == AST_BLOCK &&
           n->a->list.len >= 2;
}

static const char *jsg_emit_extern_call(jg_t *g, const char *extname, ast_node_t **args,
                                        size_t nargs)
{
    cg_t *cg = &g->cg;
    const char *rendered[12];
    {
        size_t i = 0;
        for (; i < nargs && i < 12; i++) {
            if (jsg_should_hoist(args[i])) {
                const char *name =
                    jsg_fresh(g, jsg_handler_base(g, extname, args, nargs));
                jsg_local(g, name, false);
                cg_line(cg, "const %s = %s;", name, jsg_lambda(g, args[i]));
                rendered[i] = name;
            } else {
                rendered[i] = jsg_expr_p(g, args[i], 0);
            }
        }
    }
    {
        const js_host_t *h = js_host_lookup(extname);
        if (!h) {
            sb_t b;
            sb_init(&b);
            sb_puts(&b, cg_cident(cg, extname));
            sb_putc(&b, '(');
            {
                size_t i = 0;
                for (; i < nargs && i < 12; i++) {
                    if (i) sb_puts(&b, ", ");
                    sb_puts(&b, rendered[i]);
                }
            }
            sb_putc(&b, ')');
            {
                const char *r = arena_strdup(cg->a, sb_cstr(&b));
                sb_free(&b);
                return r;
            }
        }
        {
            sb_t b;
            sb_init(&b);
            {
                const char *p = h->tmpl;
                for (; *p; p++) {
                    if (*p != '%') {
                        sb_putc(&b, *p);
                        continue;
                    }
                    p++;
                    if (*p == '%') {
                        sb_putc(&b, '%');
                    } else if (*p == '+' && p[1] >= '0' && p[1] <= '9') {
                        size_t start = (size_t)(p[1] - '0');
                        p++;
                        {
                            size_t i = start;
                            for (; i < nargs && i < 12; i++) {
                                if (i > start) sb_puts(&b, ", ");
                                sb_puts(&b, rendered[i]);
                            }
                        }
                    } else if (*p >= '0' && *p <= '9') {
                        size_t i = (size_t)(*p - '0');
                        sb_puts(&b, (i < nargs && i < 12) ? rendered[i] : "undefined");
                    } else if (*p == '\0') {
                        break;
                    }
                }
            }
            {
                const char *r = arena_strdup(cg->a, sb_cstr(&b));
                sb_free(&b);
                return r;
            }
        }
    }
}

static const char *jsg_try_inline(jg_t *g, ast_node_t *call, func_sig_t *sig,
                                  scope_t *home)
{
    ast_node_t *decl = sig ? sig->decl : NULL;
    ast_node_t *st, *inner, *callee;
    size_t nparams;
    if (!decl || decl->is_extern || decl->synthetic || decl->typarams.len) return NULL;
    if (!decl->a || decl->a->kind != AST_BLOCK || decl->a->list.len != 1) return NULL;
    st = (ast_node_t *)decl->a->list.data[0];
    if (st->kind != AST_RETURN && st->kind != AST_EXPR_STMT) return NULL;
    inner = st->a;
    if (!inner || inner->kind != AST_CALL) return NULL;
    callee = inner->a;
    if (!callee || callee->kind != AST_IDENTIFIER || !callee->name) return NULL;
    nparams = decl->list.len;
    if (nparams > 12 || inner->list.len > 12) return NULL;
    if (call->list.len > nparams) return NULL;
    {
        symbol_t *es = home ? scope_lookup(home, callee->name) : NULL;
        if (es) {
            func_sig_t *esig;
            if (es->kind != SYM_FUNC || es->overloads.len != 1) return NULL;
            esig = (func_sig_t *)es->overloads.data[0];
            if (!esig || !esig->decl || !esig->decl->is_extern) return NULL;
        } else if (!js_host_lookup(callee->name)) {
            return NULL;
        }
    }
    {
        size_t map[12];
        bool used[12];
        {
            size_t i = 0;
            for (; i < nparams; i++)
                used[i] = false;
        }
        {
            size_t k = 0;
            for (; k < inner->list.len; k++) {
                ast_node_t *arg = (ast_node_t *)inner->list.data[k];
                bool found = false;
                if (!arg || arg->kind != AST_IDENTIFIER || !arg->name) return NULL;
                {
                    size_t i = 0;
                    for (; i < nparams; i++) {
                        ast_node_t *p = (ast_node_t *)decl->list.data[i];
                        if (p->name && !strcmp(p->name, arg->name) && !used[i]) {
                            map[k] = i;
                            used[i] = true;
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) return NULL;
            }
        }
        {
            size_t i = 0;
            for (; i < nparams; i++)
                if (!used[i]) return NULL;
        }
        {
            ast_node_t *outer[12];
            ast_node_t *slots[12];
            {
                size_t i = 0;
                for (; i < nparams; i++) {
                    if (i < call->list.len)
                        outer[i] = (ast_node_t *)call->list.data[i];
                    else {
                        ast_node_t *p = (ast_node_t *)decl->list.data[i];
                        if (!p->a) return NULL;
                        outer[i] = p->a;
                    }
                }
            }
            {
                size_t k = 0;
                for (; k < inner->list.len; k++)
                    slots[k] = outer[map[k]];
            }
            return jsg_emit_extern_call(g, callee->name, slots, inner->list.len);
        }
    }
}

static const char *jsg_call_extern_direct(jg_t *g, ast_node_t *n, const char *name)
{
    ast_node_t *args[12];
    size_t i = 0;
    for (; i < n->list.len && i < 12; i++)
        args[i] = (ast_node_t *)n->list.data[i];
    return jsg_emit_extern_call(g, name, args, i);
}

static const char *jsg_print(jg_t *g, ast_node_t *n, int err)
{
    cg_t *cg = &g->cg;
    sb_t b;
    sb_init(&b);
    sb_puts(&b, err ? "console.error(" : "console.log(");
    {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            if (i) sb_puts(&b, ", ");
            sb_puts(&b, jsg_expr_p(g, (ast_node_t *)n->list.data[i], 0));
        }
    }
    sb_putc(&b, ')');
    {
        const char *r = arena_strdup(cg->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

static const char *jsg_call_ident(jg_t *g, ast_node_t *n, ast_node_t *callee)
{
    cg_t *cg = &g->cg;
    const char *nm = callee->name;
    bool is_print = !strcmp(nm, "print") || !strcmp(nm, "println");
    bool is_printerr = !strcmp(nm, "printerr") || !strcmp(nm, "printerrln");
    if (is_print || is_printerr) return jsg_print(g, n, is_printerr ? 1 : 0);
    if (!strcmp(nm, "input")) return "salam_input()";
    if (!strcmp(nm, "lang")) return cg_fmt(cg, "\"%s\"", i18n_lang());
    if (!strcmp(nm, "funcptr") && n->list.len == 1) {
        ast_node_t *fn = (ast_node_t *)n->list.data[0];
        symbol_t *fsym = scope_lookup(cg->sem->global, fn->name);
        if (fsym && fsym->overloads.len > 0) {
            func_sig_t *sig = (func_sig_t *)fsym->overloads.data[0];
            return jsg_fn_name(g, fsym->pkgname ? fsym->pkgname : cg->pkg, NULL, fn->name,
                               fsym, sig, sig && sig->decl && sig->decl->is_extern,
                               sig && sig->decl && sig->decl->synthetic);
        }
        return cg_cident(cg, fn->name);
    }
    if (!strcmp(nm, "callhandler") && n->list.len == 2)
        return cg_fmt(cg, "%s(%s)",
                      jsg_expr_p(g, (ast_node_t *)n->list.data[0], JSP_MEMBER),
                      jsg_expr_p(g, (ast_node_t *)n->list.data[1], 0));
    if (!strcmp(nm, "len") && n->list.len == 1) {
        ast_node_t *arg = (ast_node_t *)n->list.data[0];
        long klen = ast_str_lit_len(arg);
        if (klen >= 0) return cg_fmt(cg, "%ld", klen);
        return cg_fmt(cg, "%s.length", jsg_expr_p(g, arg, JSP_MEMBER));
    }
    if (!strcmp(nm, "char_code") && n->list.len == 1)
        return cg_fmt(cg, "%s.charCodeAt(0)",
                      jsg_expr_p(g, (ast_node_t *)n->list.data[0], JSP_MEMBER));
    if (!scope_lookup(cg->sem->global, nm)) {
        if (!strcmp(nm, "hash") && n->list.len == 1) {
            ast_node_t *a0 = (ast_node_t *)n->list.data[0];
            const char *ats = a0->type_str ? a0->type_str : "";
            if (!strcmp(ats, "str"))
                return cg_fmt(cg, "salam_str_hash(%s)", jsg_expr_p(g, a0, 0));
            return cg_fmt(cg, "salam_hash_int(%s)", jsg_expr_p(g, a0, 0));
        }
        if (!strcmp(nm, "sizeof") || !strcmp(nm, "spawn") || !strcmp(nm, "args") ||
            !strcmp(nm, "listdir"))
            return jsg_unsupported(g, nm);
    }
    {
        const salam_builtin_t *bi = salam_builtin_lookup(nm);
        symbol_t *fsym0 = scope_lookup(cg->sem->global, nm);
        if (bi && !fsym0)
            return cg_fmt(cg, "%s(%s)", bi->runtime, jsg_call_args(g, n, NULL));
    }
    {
        symbol_t *fsym = scope_lookup(cg->sem->global, nm);
        const char *home_pkg = NULL;
        scope_t *home = cg->sem->global;
        if (!fsym && cg->cur_fn_home) {
            symbol_t *hs = scope_lookup(cg->cur_fn_home, nm);
            if (hs && hs->kind == SYM_FUNC) {
                fsym = hs;
                home_pkg = hs->pkgname;
                home = hs->home ? hs->home : cg->cur_fn_home;
            }
        }
        if (!fsym && cg->cur_struct && cg->cur_struct->home) {
            symbol_t *hs = scope_lookup(cg->cur_struct->home, nm);
            if (hs && hs->kind == SYM_FUNC) {
                fsym = hs;
                home_pkg = hs->pkgname;
                home = hs->home ? hs->home : cg->cur_struct->home;
            }
        }
        if (fsym && (fsym->kind == SYM_VAR || fsym->kind == SYM_CONST) &&
            !local_known(cg, nm))
            return cg_fmt(cg, "%s(%s)",
                          jsg_global_ref(g, fsym->pkgname ? fsym->pkgname : cg->pkg, nm),
                          jsg_call_args(g, n, NULL));
        {
            func_sig_t *sig = fsym ? pick_overload(cg, fsym, n) : NULL;
            bool is_extern_call = sig && sig->decl && sig->decl->is_extern;
            bool inst_fn = sig && sig->decl && sig->decl->synthetic;
            if (is_extern_call) return jsg_call_extern_direct(g, n, nm);
            {
                const char *inl = jsg_try_inline(g, n, sig, home);
                if (inl) return inl;
            }
            {
                const char *fn = sig ? jsg_fn_name(g, home_pkg ? home_pkg : cg->pkg, NULL,
                                                   nm, fsym, sig, false, inst_fn)
                                     : jsg_ident(g, nm);
                return cg_fmt(cg, "%s(%s)", fn, jsg_call_args(g, n, sig));
            }
        }
    }
}

static const char *jsg_call_pkg(jg_t *g, ast_node_t *n, symbol_t *pk, ast_node_t *callee)
{
    cg_t *cg = &g->cg;
    symbol_t *fn = scope_lookup_local(pk->members, callee->name);
    func_sig_t *sig = fn ? pick_overload(cg, fn, n) : NULL;
    if (sig && sig->decl && sig->decl->is_extern)
        return jsg_call_extern_direct(g, n, callee->name);
    {
        const char *inl = jsg_try_inline(g, n, sig, pk->members);
        if (inl) return inl;
    }
    {
        bool inst = sig && sig->decl && sig->decl->synthetic;
        const char *name =
            jsg_fn_name(g, pk->pkgname, NULL, callee->name, fn, sig, false, inst);
        return cg_fmt(cg, "%s(%s)", name, jsg_call_args(g, n, sig));
    }
}

static const char *jsg_call_array(jg_t *g, ast_node_t *n, ast_node_t *obj,
                                  ast_node_t *callee)
{
    cg_t *cg = &g->cg;
    (void)n;
    if (!strcmp(callee->name, "len"))
        return cg_fmt(cg, "%s.length", jsg_expr_p(g, obj, JSP_MEMBER));
    return "0";
}

static const char *jsg_call_str(jg_t *g, ast_node_t *n, ast_node_t *obj,
                                ast_node_t *callee)
{
    cg_t *cg = &g->cg;
    const char *m = callee->name;
    if (!strcmp(m, "len")) {
        long klen = ast_str_lit_len(obj);
        if (klen >= 0) return cg_fmt(cg, "%ld", klen);
    }
    {
        const char *recv = jsg_expr_p(g, obj, JSP_MEMBER);
        const char *a0 =
            n->list.len > 0 ? jsg_expr_p(g, (ast_node_t *)n->list.data[0], 0) : "0";
        const char *a1 =
            n->list.len > 1 ? jsg_expr_p(g, (ast_node_t *)n->list.data[1], 0) : "0";
        if (!strcmp(m, "len")) return cg_fmt(cg, "%s.length", recv);
        if (!strcmp(m, "concat")) return cg_fmt(cg, "%s + %s", recv, a0);
        if (!strcmp(m, "substr")) return cg_fmt(cg, "%s.substr(%s, %s)", recv, a0, a1);
        if (!strcmp(m, "find")) return cg_fmt(cg, "%s.indexOf(%s)", recv, a0);
        if (!strcmp(m, "trim")) return cg_fmt(cg, "%s.trim()", recv);
        if (!strcmp(m, "to_int")) return cg_fmt(cg, "(parseInt(%s, 10) || 0)", recv);
        if (!strcmp(m, "to_float")) return cg_fmt(cg, "(parseFloat(%s) || 0)", recv);
        if (!strcmp(m, "split")) return cg_fmt(cg, "%s.split(%s)", recv, a0);
        return "0";
    }
}

static const char *jsg_call_method(jg_t *g, ast_node_t *n, ast_node_t *obj,
                                   ast_node_t *callee, const char *objts)
{
    cg_t *cg = &g->cg;
    char sname[96];
    symbol_t *ssym = cg_struct_of(cg, objts, sname, sizeof sname);
    symbol_t *msym = ssym ? scope_lookup_local(ssym->members, callee->name) : NULL;
    func_sig_t *sig = msym ? pick_overload(cg, msym, n) : NULL;
    const char *spkg = (ssym && ssym->pkgname) ? ssym->pkgname : cg->pkg;
    const char *fn = jsg_fn_name(g, spkg, sname, callee->name, msym, sig, false,
                                 sig && sig->decl && sig->decl->synthetic);
    const char *as = jsg_call_args(g, n, sig);
    return cg_fmt(cg, "%s(%s%s%s)", fn, jsg_expr_p(g, obj, 0), as[0] ? ", " : "", as);
}

static const char *jsg_call_impl(jg_t *g, ast_node_t *n, ast_node_t *obj,
                                 ast_node_t *callee, const char *objts, symbol_t *owner)
{
    cg_t *cg = &g->cg;
    symbol_t *msym = scope_lookup_local(owner->members, callee->name);
    func_sig_t *sig = msym ? pick_overload(cg, msym, n) : NULL;
    vec_t empty;
    vec_init(&empty);
    {
        const char *fn =
            (msym && msym->overloads.len > 1 && sig)
                ? cg_mangle_ti(cg, objts, callee->name, &sig->params)
                : jsg_ident(g, cg_fmt(cg, "%s_%s", cg_cident(cg, objts), callee->name));
        const char *as = jsg_call_args(g, n, sig);
        return cg_fmt(cg, "%s(%s%s%s)", fn, jsg_expr_p(g, obj, 0), as[0] ? ", " : "", as);
    }
}

static const char *jsg_call_member(jg_t *g, ast_node_t *n, ast_node_t *callee)
{
    cg_t *cg = &g->cg;
    ast_node_t *obj = callee->a;
    if (obj && obj->kind == AST_IDENTIFIER && !local_known(cg, obj->name)) {
        symbol_t *pk = scope_lookup(cg->sem->global, obj->name);
        if ((!pk || pk->kind != SYM_PACKAGE) && cg->cur_struct && cg->cur_struct->home)
            pk = scope_lookup(cg->cur_struct->home, obj->name);
        if ((!pk || pk->kind != SYM_PACKAGE) && cg->cur_fn_home)
            pk = scope_lookup(cg->cur_fn_home, obj->name);
        if (pk && pk->kind == SYM_PACKAGE) return jsg_call_pkg(g, n, pk, callee);
    }
    {
        const char *objts = obj->type_str ? obj->type_str : "";
        {
            symbol_t *impl = scope_lookup(cg->sem->global, impl_owner_key(cg->a, objts));
            if (impl && impl->kind == SYM_TYPEIMPL &&
                scope_lookup_local(impl->members, callee->name))
                return jsg_call_impl(g, n, obj, callee, objts, impl);
        }
        if (!strncmp(objts, "dyn ", 4)) return jsg_unsupported(g, "dyn interface call");
        if (!strcmp(objts, "File*")) return jsg_unsupported(g, "File");
        if (!strcmp(objts, "str")) return jsg_call_str(g, n, obj, callee);
        if (cg_is_slice_ts(objts) || (objts && strchr(objts, '[')))
            return jsg_call_array(g, n, obj, callee);
        return jsg_call_method(g, n, obj, callee, objts);
    }
}

const char *jsg_call(jg_t *g, ast_node_t *n)
{
    ast_node_t *callee = n->a;
    if (callee && callee->type_str && !strncmp(callee->type_str, "func(", 5) &&
        callee->kind != AST_IDENTIFIER)
        return cg_fmt(&g->cg, "(%s)(%s)", jsg_expr_p(g, callee, 0),
                      jsg_call_args(g, n, NULL));
    if (callee && callee->kind == AST_IDENTIFIER && callee->type_str &&
        !strncmp(callee->type_str, "func(", 5) && local_known(&g->cg, callee->name))
        return cg_fmt(&g->cg, "%s(%s)", jsg_local_ref(g, callee->name),
                      jsg_call_args(g, n, NULL));
    if (callee && callee->kind == AST_IDENTIFIER) return jsg_call_ident(g, n, callee);
    if (callee && callee->kind == AST_MEMBER) return jsg_call_member(g, n, callee);
    return "undefined";
}

static bool jsg_lambda_ret_void(const ast_node_t *n)
{
    const char *ts = n->type_str;
    size_t len;
    if (!ts) return false;
    len = strlen(ts);
    return len >= 5 && !strcmp(ts + len - 5, " void");
}

static ast_node_t *jsg_lambda_expr_body(ast_node_t *n)
{
    if (n->a && n->a->kind != AST_BLOCK) return n->a;
    if (n->a && n->a->kind == AST_BLOCK && n->a->list.len == 1) {
        ast_node_t *st = (ast_node_t *)n->a->list.data[0];
        if (st->kind == AST_RETURN && st->a) return st->a;
        if (st->kind == AST_EXPR_STMT && jsg_lambda_ret_void(n)) return st->a;
    }
    return NULL;
}

static bool jsg_caps_immutable(jg_t *g, ast_node_t *n)
{
    size_t i = 0;
    for (; i < n->captures.len; i++) {
        ast_node_t *c = (ast_node_t *)n->captures.data[i];
        if (!c->name || !local_known(&g->cg, c->name)) return false;
        if (jsg_local_is_mut(g, c->name)) return false;
    }
    return true;
}

static const char *jsg_lambda(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    bool plain = n->captures.len == 0 || jsg_caps_immutable(g, n);
    sb_t caps, capvals;
    sb_t params;
    sb_init(&caps);
    sb_init(&capvals);
    if (!plain) {
        size_t i = 0;
        for (; i < n->captures.len; i++) {
            ast_node_t *c = (ast_node_t *)n->captures.data[i];
            if (i) sb_puts(&capvals, ", ");
            sb_puts(&capvals, jsg_expr_p(g, c, 0));
        }
    }
    sb_init(&params);
    {
        const char *core;
        size_t mark = cg->locals.len;
        vec_t saved_defers = cg->fn_defers;
        vec_init(&cg->fn_defers);
        if (!plain) {
            size_t i = 0;
            for (; i < n->captures.len; i++) {
                ast_node_t *c = (ast_node_t *)n->captures.data[i];
                const char *nm = jsg_local(g, c->name, false);
                if (i) sb_puts(&caps, ", ");
                sb_puts(&caps, nm);
            }
        }
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                ast_node_t *p = (ast_node_t *)n->list.data[i];
                const char *nm = jsg_local(g, p->name, false);
                if (i) sb_puts(&params, ", ");
                sb_puts(&params, nm);
            }
        }
        {
            ast_node_t *expr = jsg_lambda_expr_body(n);
            if (expr) {
                const char *body = jsg_expr_p(g, expr, JSP_LAMBDA);
                if (expr->kind == AST_STRUCT_LIT) body = cg_fmt(cg, "(%s)", body);
                core = cg_fmt(cg, "(%s) => %s", sb_cstr(&params), body);
            } else {
                sb_t *saved = cg->c;
                sb_t body;
                const char *saved_mres = cg->match_result_tmp;
                const char *saved_mlbl = cg->match_end_label;
                sb_init(&body);
                cg->c = &body;
                cg->indent++;
                cg->match_result_tmp = NULL;
                cg->match_end_label = NULL;
                if (n->a && n->a->kind == AST_BLOCK) {
                    jsg_block(g, n->a);
                    jsg_emit_defers(g);
                } else if (n->a) {
                    cg_line(cg, "return %s;", jsg_expr_p(g, n->a, 0));
                }
                cg->match_result_tmp = saved_mres;
                cg->match_end_label = saved_mlbl;
                cg->indent--;
                cg->c = saved;
                {
                    sb_t pad;
                    sb_init(&pad);
                    {
                        int i = 0;
                        for (; i < cg->indent; i++)
                            sb_puts(&pad, "    ");
                    }
                    core = cg_fmt(cg, "(%s) => {\n%s%s}", sb_cstr(&params),
                                  sb_cstr(&body), sb_cstr(&pad));
                    sb_free(&pad);
                    sb_free(&body);
                }
            }
        }
        jsg_scope_reset(g, mark);
        cg->fn_defers = saved_defers;
        if (!plain)
            core =
                cg_fmt(cg, "((%s) => %s)(%s)", sb_cstr(&caps), core, sb_cstr(&capvals));
        sb_free(&caps);
        sb_free(&capvals);
        sb_free(&params);
        return core;
    }
}

static const char *jsg_struct_lit(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    symbol_t *ssym = struct_by_name(cg, n->name);
    sb_t b;
    sb_init(&b);
    sb_puts(&b, "{ ");
    {
        bool first = true;
        if (ssym) {
            size_t i = 0;
            for (; i < ssym->members->symbols.len; i++) {
                symbol_t *f = (symbol_t *)ssym->members->symbols.data[i];
                ast_node_t *provided = NULL;
                if (f->kind != SYM_FIELD) continue;
                {
                    size_t j = 0;
                    for (; j < n->list.len; j++) {
                        ast_node_t *fi = (ast_node_t *)n->list.data[j];
                        if (fi->name && !strcmp(fi->name, f->name)) {
                            provided = fi;
                            break;
                        }
                    }
                }
                {
                    const char *val =
                        provided ? jsg_expr_p(g, provided->a, 0)
                        : (f->decl && f->decl->a)
                            ? jsg_expr_p(g, f->decl->a, 0)
                            : jsg_zero(g, f->type ? type_to_string(cg->sem->tc, f->type)
                                                  : NULL);
                    if (!first) sb_puts(&b, ", ");
                    sb_puts(&b, cg_fmt(cg, "%s: %s", cg_cident(cg, f->name), val));
                    first = false;
                }
            }
        } else {
            size_t j = 0;
            for (; j < n->list.len; j++) {
                ast_node_t *fi = (ast_node_t *)n->list.data[j];
                if (!first) sb_puts(&b, ", ");
                sb_puts(&b, cg_fmt(cg, "%s: %s", cg_cident(cg, fi->name),
                                   jsg_expr_p(g, fi->a, 0)));
                first = false;
            }
        }
    }
    sb_puts(&b, " }");
    {
        const char *r = arena_strdup(cg->a, sb_cstr(&b));
        sb_free(&b);
        return r;
    }
}

const char *jsg_expr(jg_t *g, ast_node_t *n)
{
    return jsg_expr_p(g, n, 0);
}

const char *jsg_match_arm_cond(jg_t *g, ast_node_t *arm, const char *subj_var,
                               const char *subj_ts)
{
    cg_t *cg = &g->cg;
    bool is_variant = subj_ts && !strncmp(subj_ts, "Variant<", 8);
    sb_t b;
    const char *r;
    sb_init(&b);
    if (arm->op == TK_KW_ELSE) {
        sb_puts(&b, "true");
    } else {
        size_t i = 0;
        for (; i < arm->list.len; i++) {
            ast_node_t *pat = (ast_node_t *)arm->list.data[i];
            if (i) sb_puts(&b, " || ");
            if (is_variant)
                sb_puts(&b,
                        cg_fmt(cg, "(%s.tag === %d)", subj_var, (int)pat->value.as.i));
            else
                sb_puts(&b, cg_fmt(cg, "(%s === %s)", subj_var, jsg_expr(g, pat->a)));
        }
    }
    r = cg_fmt(cg, "%s", sb_cstr(&b));
    sb_free(&b);
    return r;
}

static bool jsg_list_has_break_continue(ast_node_t *block);

static bool jsg_stmt_has_break_continue(ast_node_t *n)
{
    if (!n) return false;
    switch (n->kind) {
    case AST_BREAK:
    case AST_CONTINUE:
        return true;
    case AST_BLOCK:
        return jsg_list_has_break_continue(n);
    case AST_IF:
        if (jsg_list_has_break_continue(n->b)) return true;
        if (n->c)
            return n->c->kind == AST_IF ? jsg_stmt_has_break_continue(n->c)
                                        : jsg_list_has_break_continue(n->c);
        return false;
    case AST_MATCH: {
        size_t i = 0;
        for (; i < n->list.len; i++) {
            ast_node_t *arm = (ast_node_t *)n->list.data[i];
            if (jsg_list_has_break_continue(arm->b)) return true;
        }
        return false;
    }
    /* break/continue inside a nested loop targets that loop, not any
     * match expression enclosing it, so don't recurse into loop bodies. */
    case AST_UNTIL:
    case AST_REPEAT:
    case AST_EACH:
    case AST_FOR:
        return false;
    default:
        return false;
    }
}

static bool jsg_list_has_break_continue(ast_node_t *block)
{
    if (!block) return false;
    size_t i = 0;
    for (; i < block->list.len; i++)
        if (jsg_stmt_has_break_continue((ast_node_t *)block->list.data[i])) return true;
    return false;
}

static const char *jsg_match_expr(jg_t *g, ast_node_t *n)
{
    cg_t *cg = &g->cg;
    int t = ++cg->tmpn;
    const char *subj_var = cg_fmt(cg, "__msubj%d", t);
    bool hoist = jsg_stmt_has_break_continue(n);

    if (hoist) {
        /* An arm contains break/continue meant for a loop enclosing this
         * match expression. Wrapping arms in an IIFE (the usual path below)
         * would make those statements target the arrow function instead,
         * which is invalid JS. Emit the arms as plain statements hoisted
         * in place, using a labeled block + break for the arm's value
         * instead of an arrow-function return. */
        const char *res_var = cg_fmt(cg, "__mres%d", t);
        const char *lbl = cg_fmt(cg, "__mlbl%d", t);
        const char *saved_tmp = cg->match_result_tmp;
        const char *saved_lbl = cg->match_end_label;
        cg_line(cg, "let %s;", res_var);
        cg_line(cg, "%s: {", lbl);
        cg->indent++;
        cg_line(cg, "const %s = %s;", subj_var, jsg_expr(g, n->a));
        cg->match_result_tmp = res_var;
        cg->match_end_label = lbl;
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                ast_node_t *arm = (ast_node_t *)n->list.data[i];
                const char *cond = jsg_match_arm_cond(g, arm, subj_var, n->a->type_str);
                cg_line(cg, "%sif (%s) {", i ? "} else " : "", cond);
                cg->indent++;
                jsg_block(g, arm->b);
                cg->indent--;
            }
        }
        if (n->list.len) cg_line(cg, "}");
        cg->match_result_tmp = saved_tmp;
        cg->match_end_label = saved_lbl;
        cg->indent--;
        cg_line(cg, "}");
        return res_var;
    }
    {
        sb_t *saved_out = cg->c;
        sb_t inner;
        int saved_indent = cg->indent;
        size_t mark = cg->locals.len;
        const char *body;
        sb_init(&inner);
        cg->c = &inner;
        cg->indent = 0;
        cg_line(cg, "const %s = %s;", subj_var, jsg_expr(g, n->a));
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                ast_node_t *arm = (ast_node_t *)n->list.data[i];
                const char *cond = jsg_match_arm_cond(g, arm, subj_var, n->a->type_str);
                cg_line(cg, "%sif (%s) {", i ? "} else " : "", cond);
                cg->indent++;
                jsg_block(g, arm->b);
                cg->indent--;
            }
        }
        if (n->list.len) cg_line(cg, "}");
        cg->c = saved_out;
        cg->indent = saved_indent;
        cg->locals.len = mark;
        body = cg_fmt(cg, "%s", sb_cstr(&inner));
        sb_free(&inner);
        return cg_fmt(cg, "(() => {\n%s})()", body);
    }
}

const char *jsg_expr_p(jg_t *g, ast_node_t *n, int minprec)
{
    cg_t *cg = &g->cg;
    if (!n) return "0";
    switch (n->kind) {
    case AST_LITERAL:
        switch (n->op) {
        case TK_INT: {
            unsigned long long u = (unsigned long long)n->value.as.i;
            bool uns = n->type_str && n->type_str[0] == 'u';
            if (!uns && u > 9223372036854775807ULL)
                return cg_fmt(cg, "(%lld)", (long long)u);
            return cg_fmt(cg, "%llu", u);
        }
        case TK_FLOAT: {
            char buf[64];
            sal_snprintf(buf, sizeof buf, "%.17g", n->value.as.f);
            return cg_fmt(cg, "%s", buf);
        }
        case TK_STRING:
        case TK_TRIPLE_STRING:
        case TK_RAW_STRING:
        case TK_UTF8_CHAR:
            return jsg_escape(g, n->value.as.s ? n->value.as.s : "");
        case TK_CHAR:
            return cg_fmt(cg, "%llu", (unsigned long long)n->value.as.i);
        case TK_KW_TRUE:
            return "true";
        case TK_KW_FALSE:
            return "false";
        case TK_KW_NULL:
            return "null";
        default:
            return "0";
        }
    case AST_IDENTIFIER:
        if (local_known(cg, n->name)) return jsg_local_ref(g, n->name);
        if (cg->cur_struct && cg->cur_struct->members) {
            symbol_t *f = scope_lookup_local(cg->cur_struct->members, n->name);
            if (f && f->kind == SYM_FIELD)
                return cg_fmt(cg, "__self.%s", cg_cident(cg, n->name));
        }
        {
            symbol_t *s = scope_lookup(cg->sem->global, n->name);
            if (!s && cg->cur_fn_home) s = scope_lookup(cg->cur_fn_home, n->name);
            if (s && (s->kind == SYM_VAR || s->kind == SYM_CONST))
                return jsg_global_ref(g, s->pkgname ? s->pkgname : cg->pkg, n->name);
            if (s && s->kind == SYM_FUNC && s->overloads.len) {
                func_sig_t *sig = (func_sig_t *)s->overloads.data[0];
                if (sig)
                    return jsg_fn_name(g, s->pkgname ? s->pkgname : cg->pkg, NULL,
                                       n->name, s, sig, sig->decl && sig->decl->is_extern,
                                       sig->decl && sig->decl->synthetic);
            }
        }
        return jsg_ident(g, n->name);
    case AST_THIS:
        return "__self";
    case AST_BINARY: {
        if (n->a && n->a->type_str) {
            char sname[96];
            symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
            if (ssym) {
                bool neg_eq = (n->op == TK_NE && n->synthetic);
                const char *mname = neg_eq ? "operator_eq" : cg_op_method(n->op);
                if (mname) {
                    const char *call =
                        jsg_emit_op_call(g, n->a, ssym, sname, mname, n->b);
                    if (call) return neg_eq ? cg_fmt(cg, "!%s", call) : call;
                }
            }
        }
        if (n->op == TK_POWER)
            return cg_fmt(cg, "Math.pow(%s, %s)", jsg_expr_p(g, n->a, 0),
                          jsg_expr_p(g, n->b, 0));
        if (n->op == TK_SLASH && cg_is_int_typestr(n->type_str))
            return cg_fmt(cg, "Math.trunc(%s / %s)", jsg_expr_p(g, n->a, 13),
                          jsg_expr_p(g, n->b, 14));
        if (n->op == TK_PLUS && n->type_str && !strcmp(n->type_str, "str")) {
            const char *t = jsg_template_concat(g, n);
            if (t) return t;
        }
        {
            int own = jsg_binprec(n->op);
            const char *s = cg_fmt(cg, "%s %s %s", jsg_expr_p(g, n->a, own),
                                   jsg_op(n->op), jsg_expr_p(g, n->b, own + 1));
            return jsg_wrap(g, s, own, minprec);
        }
    }
    case AST_TERNARY: {
        const char *s =
            cg_fmt(cg, "%s ? %s : %s", jsg_expr_p(g, n->a, JSP_TERNARY + 1),
                   jsg_expr_p(g, n->b, JSP_LAMBDA), jsg_expr_p(g, n->c, JSP_TERNARY));
        return jsg_wrap(g, s, JSP_TERNARY, minprec);
    }
    case AST_UNARY: {
        if (n->a && n->a->type_str) {
            char sname[96];
            symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
            if (ssym) {
                const char *mname = (n->op == TK_MINUS) ? "operator_sub"
                                    : (n->op == TK_NOT) ? "operator_not"
                                                        : NULL;
                if (mname) {
                    const char *call =
                        jsg_emit_op_call(g, n->a, ssym, sname, mname, NULL);
                    if (call) return call;
                }
            }
        }
        {
            const char *s =
                cg_fmt(cg, "%s%s", cg_op(n->op), jsg_expr_p(g, n->a, JSP_UNARY));
            return jsg_wrap(g, s, JSP_UNARY, minprec);
        }
    }
    case AST_INCDEC: {
        const char *opc = (n->op == TK_PLUS_PLUS) ? "++" : "--";
        const char *operand = jsg_expr_p(g, n->a, JSP_MEMBER);
        const char *s = n->is_prefix ? cg_fmt(cg, "%s%s", opc, operand)
                                     : cg_fmt(cg, "%s%s", operand, opc);
        return jsg_wrap(g, s, JSP_UNARY, minprec);
    }
    case AST_CAST: {
        if (n->a && (n->a->kind == AST_ARRAY_LIT || n->a->kind == AST_STRUCT_LIT))
            return jsg_expr_p(g, n->a, minprec);
        {
            const char *dts = n->type && n->type->type_str ? n->type->type_str
                              : n->type_str                ? n->type_str
                                                           : "";
            const char *sts = n->a && n->a->type_str ? n->a->type_str : "";
            if (!strcmp(dts, sts)) return jsg_expr_p(g, n->a, minprec);
            if (!strncmp(dts, "dyn ", 4)) return jsg_unsupported(g, "dyn cast");
            if (cg_is_int_typestr(dts) && dts[strlen(dts) - 1] != '*' &&
                !strchr(dts, '['))
                return cg_fmt(cg, "Math.trunc(%s)", jsg_expr_p(g, n->a, 0));
            if (!strcmp(dts, "f32") || !strcmp(dts, "f64"))
                return cg_fmt(cg, "Number(%s)", jsg_expr_p(g, n->a, 0));
            if (!strcmp(dts, "bool")) {
                const char *s = cg_fmt(cg, "!!%s", jsg_expr_p(g, n->a, JSP_UNARY));
                return jsg_wrap(g, s, JSP_UNARY, minprec);
            }
            if (!strcmp(dts, "str") && strcmp(sts, "str") != 0)
                return cg_fmt(cg, "String(%s)", jsg_expr_p(g, n->a, 0));
            return jsg_expr_p(g, n->a, minprec);
        }
    }
    case AST_CALL: {
        const char *s = jsg_call(g, n);
        return jsg_wrap(g, s, JSP_MEMBER, minprec);
    }
    case AST_MEMBER: {
        if (n->a && n->a->kind == AST_IDENTIFIER && !local_known(cg, n->a->name)) {
            symbol_t *e = scope_lookup(cg->sem->global, n->a->name);
            if (e && e->kind == SYM_ENUM && e->members) {
                symbol_t *m = scope_lookup_local(e->members, n->name);
                if (m) return cg_fmt(cg, "%lld", (long long)m->enum_value);
            }
            if (e && e->kind == SYM_PACKAGE) {
                symbol_t *m = scope_lookup_local(e->members, n->name);
                if (m && (m->kind == SYM_CONST || m->kind == SYM_VAR))
                    return jsg_global_ref(g, e->pkgname ? e->pkgname : n->a->name,
                                          n->name);
                if (m && m->decl && m->decl->a) return jsg_expr_p(g, m->decl->a, minprec);
            }
        }
        return cg_fmt(cg, "%s.%s", jsg_expr_p(g, n->a, JSP_MEMBER),
                      cg_cident(cg, n->name));
    }
    case AST_INDEX: {
        if (n->a && n->a->type_str) {
            char sname[96];
            symbol_t *ssym = cg_struct_of(cg, n->a->type_str, sname, sizeof sname);
            if (ssym) {
                symbol_t *m = scope_lookup_local(ssym->members, "operator_index");
                if (m && m->kind == SYM_METHOD) {
                    const char *call =
                        jsg_emit_op_call(g, n->a, ssym, sname, "operator_index", n->b);
                    if (call) return call;
                }
            }
        }
        if (n->a && n->a->type_str && !strcmp(n->a->type_str, "str"))
            return cg_fmt(cg, "%s.charCodeAt(%s)", jsg_expr_p(g, n->a, JSP_MEMBER),
                          jsg_expr_p(g, n->b, 0));
        return cg_fmt(cg, "%s[%s]", jsg_expr_p(g, n->a, JSP_MEMBER),
                      jsg_expr_p(g, n->b, 0));
    }
    case AST_SLICE: {
        const char *base = jsg_expr_p(g, n->a, JSP_MEMBER);
        const char *lo = n->b ? jsg_expr_p(g, n->b, 0) : "0";
        if (n->c) return cg_fmt(cg, "%s.slice(%s, %s)", base, lo, jsg_expr_p(g, n->c, 0));
        return cg_fmt(cg, "%s.slice(%s)", base, lo);
    }
    case AST_MATCH:
        return jsg_match_expr(g, n);
    case AST_VARIANT_BOX:
        return cg_fmt(cg, "({ tag: %d, value: %s })", (int)n->value.as.i,
                      jsg_expr(g, n->a));
    case AST_VARIANT_UNWRAP:
        return cg_fmt(cg, "(%s).value", jsg_expr(g, n->a));
    case AST_LAMBDA: {
        const char *s = jsg_lambda(g, n);
        return jsg_wrap(g, s, JSP_LAMBDA, minprec);
    }
    case AST_STRUCT_LIT:
        return jsg_struct_lit(g, n);
    case AST_ARRAY_LIT: {
        sb_t b;
        sb_init(&b);
        sb_putc(&b, '[');
        {
            size_t i = 0;
            for (; i < n->list.len; i++) {
                if (i) sb_puts(&b, ", ");
                sb_puts(&b, jsg_expr_p(g, (ast_node_t *)n->list.data[i], 0));
            }
        }
        sb_putc(&b, ']');
        {
            const char *r = arena_strdup(cg->a, sb_cstr(&b));
            sb_free(&b);
            return r;
        }
    }
    default:
        return "0";
    }
}
