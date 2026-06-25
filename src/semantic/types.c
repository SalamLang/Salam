#include "core/prelude.h"
#include "semantic/types.h"

type_ctx_t *type_ctx_new(arena_t *a)
{
    type_ctx_t *tc = (type_ctx_t *)arena_alloc(a, sizeof(*tc));
    tc->a = a;
    vec_init(&tc->arrays);
    vec_init(&tc->ptrs);
    { int k = 0; for (; k <= TY_F64; k++) {
        type_t *t = (type_t *)arena_alloc(a, sizeof(*t));
        memset(t, 0, sizeof(*t));
        t->kind = (type_kind_t)k;
        tc->prims[k] = t;
    } }
    return tc;
}
type_t *type_prim(type_ctx_t *tc, type_kind_t kind)
{
    if (kind < 0 || kind > TY_F64) return tc->prims[TY_ERROR];
    return tc->prims[kind];
}
type_t *type_array(type_ctx_t *tc, type_t *elem, size_t length)
{
    { size_t i = 0; for (; i < tc->arrays.len; i++) {
        type_t *t = (type_t *)tc->arrays.data[i];
        if (t->elem == elem && t->length == length) return t;
    } }
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_ARRAY; t->elem = elem; t->length = length;
    vec_push(tc->a, &tc->arrays, t);
    return t;
}
type_t *type_ptr(type_ctx_t *tc, type_t *pointee)
{
    { size_t i = 0; for (; i < tc->ptrs.len; i++) {
        type_t *t = (type_t *)tc->ptrs.data[i];
        if (t->pointee == pointee) return t;
    } }
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_PTR; t->pointee = pointee;
    vec_push(tc->a, &tc->ptrs, t);
    return t;
}
type_t *type_struct(type_ctx_t *tc, void *decl, const char *name)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_STRUCT; t->decl = decl; t->name = name;
    return t;
}
type_t *type_enum(type_ctx_t *tc, void *decl, const char *name)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_ENUM; t->decl = decl; t->name = name;
    return t;
}
type_t *type_dyn(type_ctx_t *tc, void *iface_decl, const char *name)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_DYN; t->decl = iface_decl; t->name = name;
    return t;
}

static type_t *type_kv(type_ctx_t *tc, type_kind_t kind, type_t *key, type_t *val)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = kind; t->key = key; t->elem = val;
    return t;
}
type_t *type_map(type_ctx_t *tc, type_t *key, type_t *val)      { return type_kv(tc, TY_MAP, key, val); }
type_t *type_map_iter(type_ctx_t *tc, type_t *key, type_t *val) { return type_kv(tc, TY_MAP_ITER, key, val); }
type_t *type_vec(type_ctx_t *tc, type_t *elem)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_VEC; t->elem = elem;
    return t;
}
type_t *type_file(type_ctx_t *tc)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_FILE; t->name = "File";
    return t;   
}
type_t *type_func(type_ctx_t *tc, type_t *ret, const vec_t *params)
{
    type_t *t = (type_t *)arena_alloc(tc->a, sizeof(*t));
    memset(t, 0, sizeof(*t));
    t->kind = TY_FUNC;
    t->elem = ret;               
    vec_init(&t->params);
    if (params)
        { size_t i = 0; for (; i < params->len; i++)
            vec_push(tc->a, &t->params, params->data[i]); }
    return t;
}

int type_prim_kind_from_name(const char *name)
{
    struct { const char *n; type_kind_t k; } tab[] = {
        {"void",TY_VOID},{"bool",TY_BOOL},{"char",TY_CHAR},{"str",TY_STR},
        {"i8",TY_I8},{"i16",TY_I16},{"i32",TY_I32},{"int",TY_I32},{"i64",TY_I64},
        {"u8",TY_U8},{"u16",TY_U16},{"u32",TY_U32},{"uint",TY_U32},{"u64",TY_U64},
        {"f32",TY_F32},{"float",TY_F32},{"f64",TY_F64},
        
        {"تهی",TY_VOID},{"منطقی",TY_BOOL},{"نویسه",TY_CHAR},{"رشته",TY_STR},
        {"صحیح۸",TY_I8},{"صحیح۱۶",TY_I16},{"صحیح۳۲",TY_I32},{"صحیح",TY_I32},{"صحیح۶۴",TY_I64},
        {"صحیح8",TY_I8},{"صحیح16",TY_I16},{"صحیح32",TY_I32},{"صحیح64",TY_I64},
        {"طبیعی۸",TY_U8},{"طبیعی۱۶",TY_U16},{"طبیعی۳۲",TY_U32},{"طبیعی",TY_U32},{"طبیعی۶۴",TY_U64},
        {"طبیعی8",TY_U8},{"طبیعی16",TY_U16},{"طبیعی32",TY_U32},{"طبیعی64",TY_U64},
        {"اعشاری۳۲",TY_F32},{"اعشاری",TY_F32},{"اعشاری۶۴",TY_F64},
        {"اعشاری32",TY_F32},{"اعشاری64",TY_F64},
    };
    { size_t i = 0; for (; i < sizeof(tab)/sizeof(tab[0]); i++)
        if (strcmp(name, tab[i].n) == 0) return (int)tab[i].k; }
    return -1;
}

bool type_is_error(const type_t *t)   { return !t || t->kind == TY_ERROR; }

bool type_is_integer(const type_t *t) { return t && t->kind >= TY_I8 && t->kind <= TY_U64; }

bool type_is_float(const type_t *t)   { return t && (t->kind == TY_F32 || t->kind == TY_F64); }

bool type_is_numeric(const type_t *t) { return type_is_integer(t) || type_is_float(t); }

bool type_is_signed(const type_t *t)  { return t && t->kind >= TY_I8 && t->kind <= TY_I64; }

static int int_rank(type_kind_t k)
{
    switch (k) {
        case TY_I8: case TY_U8: return 1;
        case TY_I16:case TY_U16:return 2;
        case TY_I32:case TY_U32:return 3;
        case TY_I64:case TY_U64:return 4;
        default: return 0;
    }
}

bool type_equiv(const type_t *a, const type_t *b)
{
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->kind != b->kind) return false;
    switch (a->kind) {
        case TY_ARRAY:  return a->length == b->length && type_equiv(a->elem, b->elem);
        case TY_PTR:    return type_equiv(a->pointee, b->pointee);
        case TY_MAP:
        case TY_MAP_ITER: return type_equiv(a->key, b->key) && type_equiv(a->elem, b->elem);
        case TY_VEC:    return type_equiv(a->elem, b->elem);
        case TY_STRUCT:
        case TY_ENUM:
        case TY_DYN:
            if (a->decl == b->decl) return true;
            
            return a->name && b->name && strcmp(a->name, b->name) == 0;
        case TY_FUNC: {
            if (!type_equiv(a->elem, b->elem)) return false;   
            if (a->params.len != b->params.len) return false;
            { size_t i = 0; for (; i < a->params.len; i++)
                if (!type_equiv((type_t *)a->params.data[i], (type_t *)b->params.data[i]))
                    return false; }
            return true;
        }
        default:        return true;     
    }
}

static bool type_implicit(const type_t *src, const type_t *dst)
{
    if (!src || !dst) return false;
    if (src->kind == TY_NULL && dst->kind == TY_PTR) return true;
    if (src->kind == TY_ENUM && dst->kind == TY_I32) return true;
    
    if (dst->kind == TY_DYN && src->kind == TY_STRUCT) return true;
    if (type_is_signed(src) && type_is_signed(dst))
        return int_rank(src->kind) <= int_rank(dst->kind);
    if (type_is_integer(src) && !type_is_signed(src) &&
        type_is_integer(dst) && !type_is_signed(dst))
        return int_rank(src->kind) <= int_rank(dst->kind);
    if (type_is_integer(src) && type_is_float(dst)) return true;       
    if (src->kind == TY_F32 && dst->kind == TY_F64) return true;
    return false;
}

bool type_assignable(const type_t *dst, const type_t *src)
{
    if (type_is_error(dst) || type_is_error(src)) return true;   
    if (type_equiv(dst, src)) return true;
    return type_implicit(src, dst);
}

bool type_castable(const type_t *dst, const type_t *src)
{
    if (type_is_error(dst) || type_is_error(src)) return true;
    if (type_equiv(dst, src)) return true;
    if (dst->kind == TY_PTR && src->kind == TY_PTR) return true;  
    
    if ((dst->kind == TY_STR && src->kind == TY_PTR) ||
        (dst->kind == TY_PTR && src->kind == TY_STR)) return true;
    
    if ((dst->kind == TY_PTR && type_is_integer(src)) ||
        (src->kind == TY_PTR && type_is_integer(dst))) return true;
    bool sn = type_is_numeric(src) || src->kind == TY_BOOL || src->kind == TY_CHAR || src->kind == TY_ENUM;
    bool dn = type_is_numeric(dst) || dst->kind == TY_BOOL || dst->kind == TY_CHAR || dst->kind == TY_ENUM;
    if (sn && dn) return true;       
    return false;
}
type_t *type_common_arith(type_ctx_t *tc, type_t *a, type_t *b)
{
    if (!type_is_numeric(a) || !type_is_numeric(b)) return NULL;
    if (a->kind == TY_F64 || b->kind == TY_F64) return type_prim(tc, TY_F64);
    if (a->kind == TY_F32 || b->kind == TY_F32) return type_prim(tc, TY_F32);
    
    bool sa = type_is_signed(a), sb = type_is_signed(b);
    if (sa != sb) return NULL;       
    return int_rank(a->kind) >= int_rank(b->kind) ? a : b;
}

const char *type_to_string(type_ctx_t *tc, const type_t *t)
{
    if (!t) return "<null>";
    switch (t->kind) {
        case TY_ERROR: return "<error>";
        case TY_VOID:  return "void";
        case TY_BOOL:  return "bool";
        case TY_CHAR:  return "char";
        case TY_STR:   return "str";
        case TY_NULL:  return "null";
        case TY_I8:return "i8"; case TY_I16:return "i16"; case TY_I32:return "i32"; case TY_I64:return "i64";
        case TY_U8:return "u8"; case TY_U16:return "u16"; case TY_U32:return "u32"; case TY_U64:return "u64";
        case TY_F32:return "f32"; case TY_F64:return "f64";
        case TY_STRUCT: case TY_ENUM: return t->name ? t->name : "<anon>";
        case TY_DYN: {
            char buf[128]; snprintf(buf, sizeof(buf), "dyn %s", t->name ? t->name : "<anon>");
            return arena_strdup(tc->a, buf);
        }
        case TY_PTR: {
            const char *inner = type_to_string(tc, t->pointee);
            char buf[128]; snprintf(buf, sizeof(buf), "%s*", inner);
            return arena_strdup(tc->a, buf);
        }
        case TY_ARRAY: {
            char dims[128] = ""; size_t dn = 0;
            const type_t *cur = t;
            while (cur && cur->kind == TY_ARRAY) {
                if (cur->length) dn += (size_t)snprintf(dims + dn, sizeof(dims) - dn, "[%zu]", cur->length);
                else             dn += (size_t)snprintf(dims + dn, sizeof(dims) - dn, "[]");
                cur = cur->elem;
            }
            char buf[160];
            snprintf(buf, sizeof(buf), "%s%s", type_to_string(tc, cur), dims);
            return arena_strdup(tc->a, buf);
        }
        case TY_MAP: case TY_MAP_ITER: {
            char buf[160];
            snprintf(buf, sizeof(buf), "%s<%s, %s>",
                     t->kind == TY_MAP ? "HashMap" : "MapIter",
                     type_to_string(tc, t->key), type_to_string(tc, t->elem));
            return arena_strdup(tc->a, buf);
        }
        case TY_VEC: {
            char buf[160];
            snprintf(buf, sizeof(buf), "Vector<%s>", type_to_string(tc, t->elem));
            return arena_strdup(tc->a, buf);
        }
        case TY_FILE: return "File";
        case TY_FUNC: {
            char buf[256]; size_t o = 0;
            o += (size_t)snprintf(buf + o, sizeof(buf) - o, "func(");
            { size_t i = 0; for (; i < t->params.len && o < sizeof(buf) - 32; i++)
                o += (size_t)snprintf(buf + o, sizeof(buf) - o, "%s%s",
                        i ? ", " : "", type_to_string(tc, (type_t *)t->params.data[i])); }
            o += (size_t)snprintf(buf + o, sizeof(buf) - o, ") %s", type_to_string(tc, t->elem));
            return arena_strdup(tc->a, buf);
        }
    }
    return "<?>";
}
