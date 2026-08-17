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
#include "semantic/sema_internal.h"
#include "i18n/i18n.h"

type_t *sema_ty(sema_t *s, type_kind_t k)
{
    return type_prim(s->tc, k);
}

type_t *sema_err_ty(sema_t *s)
{
    return type_prim(s->tc, TY_ERROR);
}

type_t *sema_decorate(sema_t *s, ast_node_t *n, type_t *t)
{
    if (n) n->type_str = type_to_string(s->tc, t);
    return t;
}

type_t *sema_vec_str(sema_t *s, const src_span_t *span)
{
    ast_node_t *tn = ast_new(s->a, AST_TYPE, span);
    tn->name = "Vector";
    tn->synthetic = true;
    ast_node_t *e = ast_new(s->a, AST_TYPE, span);
    e->name = "str";
    e->synthetic = true;
    ast_add(s->a, tn, e);
    return sema_resolve_type(s, tn);
}

symbol_t *struct_sym_of(type_t *t)
{
    if (!t) return NULL;
    if (t->kind == TY_STRUCT) return (symbol_t *)t->decl;
    if (t->kind == TY_PTR && t->pointee && t->pointee->kind == TY_STRUCT)
        return (symbol_t *)t->pointee->decl;
    return NULL;
}

static bool su_zwnj(const char *s)
{
    return (unsigned char)s[0] == 0xE2 && (unsigned char)s[1] == 0x80 &&
           (unsigned char)s[2] == 0x8C;
}

static int su_fold(const char **p)
{
    const unsigned char *s = (const unsigned char *)*p;
    if (s[0] == 0) return 0;
    if ((s[0] == 0xD9 && s[1] == 0x8A) || (s[0] == 0xDB && s[1] == 0x8C)) {
        *p += 2;
        return 0x101;
    }
    if ((s[0] == 0xD9 && s[1] == 0x83) || (s[0] == 0xDA && s[1] == 0xA9)) {
        *p += 2;
        return 0x102;
    }
    *p += 1;
    return s[0];
}

static bool su_ident_eq(const char *a, const char *b)
{
    for (;;) {
        int ka, kb;
        while (su_zwnj(a) || *a == ' ')
            a += (*a == ' ') ? 1 : 3;
        while (su_zwnj(b) || *b == ' ')
            b += (*b == ' ') ? 1 : 3;
        ka = su_fold(&a);
        kb = su_fold(&b);
        if (ka != kb) return false;
        if (ka == 0) return true;
    }
}

typedef struct {
    const char *fa, *ar, *en;
} intrinsic_name_t;

static const intrinsic_name_t k_intrinsic_types[] = {
    {"وکتور", "متجه", "Vector"},
    {"نگاشت", "خريطة", "HashMap"},
    {"پیمایشگرنگاشت", "مكررخريطة", "MapIter"},
    {"پرونده", "ملف", "File"},
    {"گوناگون", "متنوع", "Variant"},
};

static const intrinsic_name_t k_intrinsic_methods[] = {

    {"بیفزا", "أضف", "push"},
    {"دربیاور", "أخرج", "pop"},
    {"بگیر", "احصل", "get"},
    {"ارجاع", "مرجع", "ref"},
    {"بنشان", "عين", "set"},
    {"طول", "طول", "len"},
    {"ظرفیت", "سعة", "cap"},
    {"آزادکن", "حرر", "free"},

    {"درج", "ضع", "put"},
    {"دارد", "يحوي", "has"},
    {"حذف", "احذف", "remove"},
    {"اندازه", "حجم", "size"},
    {"پیمایش", "تكرار", "iter"},

    {"داردبعدی", "يوجدتالي", "has_next"},
    {"کلید", "مفتاح", "key"},
    {"مقدار", "قيمة", "value"},
    {"بعدی", "التالي", "next"},

    {"خواندن", "قراءة", "read"},
    {"خواندن خط", "قراءةسطر", "readline"},
    {"نوشتن", "كتابة", "write"},
    {"جابجایی", "انتقال", "seek"},
    {"ببند", "أغلق", "close"},

    {"پیوست", "دمج", "concat"},
    {"زیررشته", "مقطع", "substr"},
    {"بیاب", "ابحث", "find"},
    {"بشکاف", "قسم", "split"},
    {"پیراست", "شذب", "trim"},
    {"به صحیح", "إلىصحيح", "to_int"},
    {"به اعشار", "إلىعشري", "to_float"},
};

static const char *intrinsic_canon(const intrinsic_name_t *tab, size_t n,
                                   const char *name)
{
    if (!name) return name;
    {
        size_t i = 0;
        for (; i < n; i++)
            if (su_ident_eq(name, tab[i].fa) || su_ident_eq(name, tab[i].ar))
                return tab[i].en;
    }
    return name;
}

static const char *intrinsic_lang_form(const intrinsic_name_t *tab, size_t n,
                                       const char *en_name, const char *lang)
{
    if (!lang || lang[0] == 'e') return NULL;
    {
        size_t i = 0;
        for (; i < n; i++)
            if (strcmp(tab[i].en, en_name) == 0)
                return lang[0] == 'f' ? tab[i].fa : tab[i].ar;
    }
    return NULL;
}

static void check_lang_form(sema_t *s, const char *want, const char *used,
                            const src_span_t *span)
{
    if (!want || su_ident_eq(used, want)) return;
    {
        bool fa = s->lang && s->lang[0] == 'f';
        const char *msg = fa ? "identifier '%s' must be Persian in a Persian file"
                             : "identifier '%s' must be Arabic in an Arabic file";
        SERR(s, 1, span, i18n_tr(msg), used);
    }
}

const char *intrinsic_type_canon(const char *name)
{
    return intrinsic_canon(k_intrinsic_types,
                           sizeof(k_intrinsic_types) / sizeof(k_intrinsic_types[0]),
                           name);
}

const char *intrinsic_method_canon(const char *name)
{
    return intrinsic_canon(k_intrinsic_methods,
                           sizeof(k_intrinsic_methods) / sizeof(k_intrinsic_methods[0]),
                           name);
}

void sema_check_intrinsic_type_lang(sema_t *s, const char *used, const src_span_t *span)
{
    if (!s->lang || s->lang[0] == 'e' || !used) return;
    {
        const char *canon = intrinsic_canon(
            k_intrinsic_types, sizeof(k_intrinsic_types) / sizeof(k_intrinsic_types[0]),
            used);
        const char *want = intrinsic_lang_form(
            k_intrinsic_types, sizeof(k_intrinsic_types) / sizeof(k_intrinsic_types[0]),
            canon, s->lang);
        check_lang_form(s, want, used, span);
    }
}

void sema_check_intrinsic_method_lang(sema_t *s, const char *used, const char *canon,
                                      const src_span_t *span)
{
    if (!s->lang || s->lang[0] == 'e' || !used || !canon) return;
    {
        const char *want = intrinsic_lang_form(
            k_intrinsic_methods,
            sizeof(k_intrinsic_methods) / sizeof(k_intrinsic_methods[0]), canon, s->lang);
        check_lang_form(s, want, used, span);
    }
}

const char *alias_for_lang(const vec_t *aliases, const char *lang)
{
    {
        size_t i = 0;
        for (; i + 1 < aliases->len; i += 2)
            if (strcmp((const char *)aliases->data[i], lang) == 0)
                return (const char *)aliases->data[i + 1];
    }
    return NULL;
}

static bool aliases_have(const vec_t *aliases, const char *lang, const char *name)
{
    size_t i = 0;
    for (; i + 1 < aliases->len; i += 2)
        if (strcmp((const char *)aliases->data[i], lang) == 0 &&
            su_ident_eq((const char *)aliases->data[i + 1], name))
            return true;
    return false;
}

const char *scope_member_canon(sema_t *s, scope_t *members, const char *name,
                               const src_span_t *span)
{
    const char *lang = (s->lang && *s->lang) ? s->lang : i18n_lang();
    if (!name || !members) return name;
    if (strcmp(lang, "en") != 0) {
        size_t i = 0;
        for (; i < members->symbols.len; i++) {
            symbol_t *m = (symbol_t *)members->symbols.data[i];
            if (!m->decl) continue;
            if (aliases_have(&m->decl->aliases, lang, name)) return m->name;
        }
    }
    if (s->lang && s->lang[0] != 'e') {
        symbol_t *direct = scope_lookup_local(members, name);
        if (direct && direct->decl && !direct->decl->synthetic)
            check_lang_form(s, alias_for_lang(&direct->decl->aliases, s->lang), name,
                            span);
    }
    return name;
}

const char *pkg_member_canon(sema_t *s, symbol_t *pk, const char *name,
                             const src_span_t *span)
{
    pk->used = true;
    return scope_member_canon(s, pk->members, name, span);
}

const char *local_canon(sema_t *s, const char *name, const src_span_t *span)
{
    return scope_member_canon(s, s->global, name, span);
}

/*
 * Resolve an interface named either bare ("Shape") or qualified
 * ("db.Connection"), which is what `dyn ...`, a type-parameter bound and
 * `impl ... on ...` each need. A qualified name has to go through the package's
 * member scope so the @en/@fa/@ar spellings canonicalize, and it must be `pub`
 * to be nameable from outside - the same two rules sema_resolve_type() applies
 * to a qualified struct.
 *
 * Returns NULL when the name does not resolve to an interface; `*why` is then
 * set to a caller-independent reason, so each caller can keep its own wording
 * ("required after 'dyn'", "bound on type parameter 'T'", ...).
 */
symbol_t *sema_lookup_iface(sema_t *s, const char *name, const src_span_t *span,
                            const char **why)
{
    const char *dot = name ? strchr(name, '.') : NULL;
    if (why) *why = NULL;
    if (!name) return NULL;

    if (!dot) {
        symbol_t *sym = scope_lookup(s->cur, name);
        if (!sym) sym = scope_lookup(s->global, name);
        if (sym && sym->kind == SYM_INTERFACE) return sym;
        /*
         * Also the canonical "pkg_Iface" spelling. A `dyn` type is named after
         * its interface's package (sema_resolve_type), so the interface bound
         * that a dyn slot checks a concrete value against arrives in that form,
         * with no package symbol in scope to split on.
         */
        {
            /* pkg_cache is a flat [path, symbol] pair list; the symbols are
             * the odd entries. */
            size_t p = 1;
            for (; s->pkg_cache && p < s->pkg_cache->len; p += 2) {
                symbol_t *pk = (symbol_t *)s->pkg_cache->data[p];
                size_t i;
                if (!pk || pk->kind != SYM_PACKAGE || !pk->members) continue;
                i = 0;
                for (; i < pk->members->symbols.len; i++) {
                    char canon[512];
                    symbol_t *m = (symbol_t *)pk->members->symbols.data[i];
                    if (!m || m->kind != SYM_INTERFACE || !m->name) continue;
                    if (!m->pkgname || !m->pkgname[0]) continue;
                    sal_snprintf(canon, sizeof canon, "%s_%s", m->pkgname, m->name);
                    if (!strcmp(canon, name)) return m;
                }
            }
        }
        return NULL;
    }

    {
        char pname[64];
        size_t pl = (size_t)(dot - name);
        if (pl >= sizeof(pname)) pl = sizeof(pname) - 1;
        memcpy(pname, name, pl);
        pname[pl] = 0;
        symbol_t *pk = scope_lookup(s->cur, pname);
        if (!pk) pk = scope_lookup(s->global, pname);
        if (!pk || pk->kind != SYM_PACKAGE) {
            if (why) *why = "unknown package";
            return NULL;
        }
        {
            const char *mname = pkg_member_canon(s, pk, dot + 1, span);
            symbol_t *sym = scope_lookup_local(pk->members, mname);
            if (!sym || sym->kind != SYM_INTERFACE) return NULL;
            if (!sym->is_pub) {
                if (why) *why = "not exported (mark it 'pub')";
                return NULL;
            }
            return sym;
        }
    }
}

ast_node_t *sema_pure_fn(sema_t *s)
{
    if (s->cur_func && s->cur_func->decl && s->cur_func->decl->is_pure)
        return s->cur_func->decl;
    return NULL;
}

/*
 * A variable, parameter or constant may not reuse a function's name. Since a
 * bare function name is now a value (its address), a local of the same name
 * would silently take the name over for the rest of its scope and a reader
 * could no longer tell which of the two an identifier meant. Only free
 * functions are checked: methods live in a struct's member scope and are only
 * ever reachable through a receiver, so they cannot be shadowed this way.
 */
void sema_check_shadows_func(sema_t *s, const char *name, const src_span_t *span)
{
    symbol_t *f;
    if (!name || !name[0] || s->in_generic_inst) return;
    f = scope_lookup_local(s->global, name);
    if (f && f->kind == SYM_FUNC)
        SERR(s, 90, span, "'%s' is already the name of a function in this file", name);
}

/*
 * The files of a multi-file package are parsed separately and merged into a
 * single program, so a top-level node's line number only makes sense against
 * the file it was parsed from. Following that file here is what keeps a
 * sibling file's diagnostics from being reported against the package's anchor
 * file, where they would quote an unrelated source line. Returns the previous
 * file for the caller to restore.
 */
const char *sema_use_decl_file(sema_t *s, const ast_node_t *d)
{
    const char *save = s->file;
    if (d && d->src_file) s->file = d->src_file;
    return save;
}
