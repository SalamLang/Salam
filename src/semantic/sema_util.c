#include "core/prelude.h"
#include "semantic/sema_internal.h"
#include "i18n/i18n.h"

type_t *sema_ty(sema_t *s, type_kind_t k) { return type_prim(s->tc, k); }
type_t *sema_err_ty(sema_t *s)            { return type_prim(s->tc, TY_ERROR); }
type_t *sema_decorate(sema_t *s, ast_node_t *n, type_t *t)
{
    if (n) n->type_str = type_to_string(s->tc, t);
    return t;
}

type_t *sema_vec_str(sema_t *s, const src_span_t *span)
{
    ast_node_t *tn = ast_new(s->a, AST_TYPE, span);
    tn->name = "Vector";
    ast_node_t *e = ast_new(s->a, AST_TYPE, span);
    e->name = "str";
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

const char *intrinsic_type_canon(const char *name)
{
    static const struct { const char *fa, *en; } tab[] = {
        { "وکتور",          "Vector"  },
        { "نگاشت",          "HashMap" },
        { "پیمایشگر_نگاشت", "MapIter" },
        { "پرونده",         "File"    },
    };
    if (!name) return name;
    { size_t i = 0; for (; i < sizeof(tab)/sizeof(tab[0]); i++)
        if (strcmp(name, tab[i].fa) == 0) return tab[i].en; }
    return name;
}

const char *intrinsic_method_canon(const char *name)
{
    static const struct { const char *fa, *en; } tab[] = {
        
        { "بیفزا",   "push" }, { "دربیاور", "pop"  }, { "بگیر",  "get" },
        { "بنشان",   "set"  }, { "طول",     "len"  }, { "ظرفیت", "cap" },
        { "آزادکن",  "free" },
        
        { "درج",     "put"  }, { "دارد",    "has"  }, { "حذف", "remove" },
        { "اندازه",  "size" }, { "پیمایش",  "iter" },
        
        { "دارد_بعدی", "has_next" }, { "کلید", "key" }, { "مقدار", "value" },
        { "بعدی",      "next"     },
        
        { "خواندن",     "read"  }, { "خواندن_خط", "readline" }, { "نوشتن", "write" },
        { "جابجایی",    "seek"  }, { "ببند",       "close"    },
        
        { "پیوست",   "concat"  }, { "زیررشته", "substr" }, { "بیاب", "find"  },
        { "بشکاف",   "split"   }, { "پیراست",  "trim"   }, { "به_صحیح", "to_int" },
        { "به_اعشار","to_float" },
    };
    if (!name) return name;
    { size_t i = 0; for (; i < sizeof(tab)/sizeof(tab[0]); i++)
        if (strcmp(name, tab[i].fa) == 0) return tab[i].en; }
    return name;
}

const char *alias_for_lang(const vec_t *aliases, const char *lang)
{
    { size_t i = 0; for (; i + 1 < aliases->len; i += 2)
        if (strcmp((const char *)aliases->data[i], lang) == 0)
            return (const char *)aliases->data[i + 1]; }
    return NULL;
}

const char *scope_member_canon(scope_t *members, const char *name)
{
    const char *lang = i18n_lang();
    if (!name || strcmp(lang, "en") == 0 || !members) return name;
    { size_t i = 0; for (; i < members->symbols.len; i++) {
        symbol_t *m = (symbol_t *)members->symbols.data[i];
        if (!m->decl) continue;
        const char *a = alias_for_lang(&m->decl->aliases, lang);
        if (a && strcmp(a, name) == 0) return m->name;   
    } }
    return name;
}

const char *pkg_member_canon(symbol_t *pk, const char *name)
{
    return scope_member_canon(pk->members, name);
}

const char *local_canon(sema_t *s, const char *name)
{
    return scope_member_canon(s->global, name);
}
