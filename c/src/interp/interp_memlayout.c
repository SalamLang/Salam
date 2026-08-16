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
 * Raw memory <-> interpreter values.
 *
 * The tree walker runs std's own Salam containers, so a Vector<T> really does
 * allocate `count * sizeof(T)` bytes and index them. For a scalar T that is a
 * plain typed load. For a struct T the bytes have to hold the fields
 * themselves, laid out exactly the way interp_sizeof_typename measures them:
 * the stride the Salam code computes and the stride the walker reads are the
 * same number, so one layout has to answer both. That is what this file owns.
 *
 * A value with no byte image at all (an array, a closure, an iterator) is kept
 * in a side table and referenced from the slot by a tagged token, never by a
 * pointer: an uninitialized slot then reads back as null instead of
 * dereferencing whatever the allocator left there.
 */

#include "interp/interp_internal.h"

typedef enum {
    SLOT_I8,
    SLOT_I16,
    SLOT_I32,
    SLOT_I64,
    SLOT_U8,
    SLOT_U16,
    SLOT_U32,
    SLOT_U64,
    SLOT_F32,
    SLOT_F64,
    SLOT_BOOL,
    SLOT_CHAR,
    SLOT_STR,
    SLOT_PTR,
    SLOT_STRUCT,
    SLOT_BOX
} slot_kind_t;

/* Tag in the top 16 bits of a box token, so a slot holding anything else
 * (zero, a small integer, a stale pointer) fails the check and reads null. */
#define MEM_BOX_TAG 0x5342ULL
#define MEM_BOX_MASK 0x0000FFFFFFFFFFFFULL

static const char *field_type_str(ast_node_t *f)
{
    return f->type ? f->type->type_str : NULL;
}

static slot_kind_t slot_kind(interp_t *I, const char *ts, ast_node_t **sdef)
{
    ast_node_t *sd;
    if (sdef) *sdef = NULL;
    if (!ts || !*ts) return SLOT_BOX;
    if (ts[strlen(ts) - 1] == '*') return SLOT_PTR;
    if (!strcmp(ts, "i8")) return SLOT_I8;
    if (!strcmp(ts, "i16")) return SLOT_I16;
    if (!strcmp(ts, "i32")) return SLOT_I32;
    if (!strcmp(ts, "i64")) return SLOT_I64;
    if (!strcmp(ts, "u8")) return SLOT_U8;
    if (!strcmp(ts, "u16")) return SLOT_U16;
    if (!strcmp(ts, "u32")) return SLOT_U32;
    if (!strcmp(ts, "u64")) return SLOT_U64;
    if (!strcmp(ts, "f32")) return SLOT_F32;
    if (!strcmp(ts, "f64")) return SLOT_F64;
    if (!strcmp(ts, "bool")) return SLOT_BOOL;
    if (!strcmp(ts, "char")) return SLOT_CHAR;
    if (!strcmp(ts, "str")) return SLOT_STR;
    sd = find_struct(I, ts);
    if (sd) {
        if (sdef) *sdef = sd;
        return SLOT_STRUCT;
    }
    return SLOT_BOX;
}

static int64_t slot_align(slot_kind_t k)
{
    switch (k) {
    case SLOT_I8:
    case SLOT_U8:
    case SLOT_BOOL:
    case SLOT_CHAR:
        return 1;
    case SLOT_I16:
    case SLOT_U16:
        return 2;
    case SLOT_I32:
    case SLOT_U32:
    case SLOT_F32:
        return 4;
    default:
        return 8;
    }
}

int64_t interp_alignof_typename(interp_t *I, const char *ts)
{
    ast_node_t *sd = NULL;
    slot_kind_t k = slot_kind(I, ts, &sd);
    int64_t maxalign = 1;
    size_t i = 0;
    if (k != SLOT_STRUCT) return slot_align(k);
    for (; i < sd->list.len; i++) {
        ast_node_t *f = (ast_node_t *)sd->list.data[i];
        int64_t al;
        if (f->kind != AST_FIELD) continue;
        al = interp_alignof_typename(I, field_type_str(f));
        if (al > maxalign) maxalign = al;
    }
    return maxalign;
}

/* Round `*off` up to where a field of type `ts` starts and return its size, so
 * every walk over a struct's bytes advances identically. */
static int64_t field_slot(interp_t *I, const char *ts, int64_t *off)
{
    int64_t al = interp_alignof_typename(I, ts);
    *off = (*off + al - 1) & ~(al - 1);
    return interp_sizeof_typename(I, ts);
}

static int64_t struct_size(interp_t *I, ast_node_t *sd)
{
    int64_t off = 0, maxalign = 1;
    size_t i = 0;
    for (; i < sd->list.len; i++) {
        ast_node_t *f = (ast_node_t *)sd->list.data[i];
        const char *fts;
        int64_t sz, al;
        if (f->kind != AST_FIELD) continue;
        fts = field_type_str(f);
        al = interp_alignof_typename(I, fts);
        sz = field_slot(I, fts, &off);
        off += sz;
        if (al > maxalign) maxalign = al;
    }
    off = (off + maxalign - 1) & ~(maxalign - 1);
    return off ? off : 8;
}

int64_t interp_sizeof_typename(interp_t *I, const char *ts)
{
    ast_node_t *sd = NULL;
    slot_kind_t k = slot_kind(I, ts, &sd);
    /* Every slot the walker stores directly is as wide as it is aligned; only
     * a struct has to be measured field by field. */
    if (k == SLOT_STRUCT) return struct_size(I, sd);
    return slot_align(k);
}

static size_t count_fields(ast_node_t *sd)
{
    size_t n = 0, i = 0;
    for (; i < sd->list.len; i++)
        if (((ast_node_t *)sd->list.data[i])->kind == AST_FIELD) n++;
    return n;
}

static uint64_t box_write(interp_t *I, value_t v)
{
    value_t *box = (value_t *)arena_alloc(I->a, sizeof *box);
    *box = v;
    vec_push(I->a, &I->mem_boxes, box);
    return (MEM_BOX_TAG << 48) | (uint64_t)I->mem_boxes.len;
}

static value_t box_read(interp_t *I, uint64_t tok)
{
    uint64_t idx = tok & MEM_BOX_MASK;
    if ((tok >> 48) != MEM_BOX_TAG || idx == 0 || idx > I->mem_boxes.len)
        return val_null();
    return *(value_t *)I->mem_boxes.data[idx - 1];
}

static value_t struct_load(interp_t *I, unsigned char *base, ast_node_t *sd)
{
    value_t sv = mk_struct(I, sd->name, sd, count_fields(sd));
    int64_t off = 0;
    size_t fi = 0, i = 0;
    for (; i < sd->list.len; i++) {
        ast_node_t *f = (ast_node_t *)sd->list.data[i];
        const char *fts;
        int64_t sz;
        if (f->kind != AST_FIELD) continue;
        fts = field_type_str(f);
        sz = field_slot(I, fts, &off);
        sv.as.st->fields[fi].name = f->name;
        sv.as.st->fields[fi].val = interp_mem_load(I, base + off, fts);
        off += sz;
        fi++;
    }
    return sv;
}

static value_t field_of(value_t v, const char *name)
{
    size_t i = 0;
    if (v.kind != VAL_STRUCT) return val_null();
    for (; i < v.as.st->nfields; i++)
        if (strcmp(v.as.st->fields[i].name, name) == 0) return v.as.st->fields[i].val;
    return val_null();
}

static void struct_store(interp_t *I, unsigned char *base, ast_node_t *sd, value_t v)
{
    int64_t off = 0;
    size_t i = 0;
    for (; i < sd->list.len; i++) {
        ast_node_t *f = (ast_node_t *)sd->list.data[i];
        const char *fts;
        int64_t sz;
        if (f->kind != AST_FIELD) continue;
        fts = field_type_str(f);
        sz = field_slot(I, fts, &off);
        /* A field the source value does not carry - a null assignment, or a
         * value of a different shape - is written as its zero rather than
         * left holding whatever the allocator handed back. */
        interp_mem_store(I, base + off, fts, field_of(v, f->name));
        off += sz;
    }
}

value_t interp_mem_load(interp_t *I, void *addr, const char *ts)
{
    ast_node_t *sd = NULL;
    unsigned char *p = (unsigned char *)addr;
    switch (slot_kind(I, ts, &sd)) {
    case SLOT_I8:
        return val_int_ty(*(int8_t *)p, ITY_I8);
    case SLOT_I16:
        return val_int_ty(*(int16_t *)p, ITY_I16);
    case SLOT_I32:
        return val_int_ty(*(int32_t *)p, ITY_I32);
    case SLOT_I64:
        return val_int_ty(*(int64_t *)p, ITY_I64);
    case SLOT_U8:
        return val_int_ty(*(uint8_t *)p, ITY_U8);
    case SLOT_U16:
        return val_int_ty(*(uint16_t *)p, ITY_U16);
    case SLOT_U32:
        return val_int_ty(*(uint32_t *)p, ITY_U32);
    case SLOT_U64:
        return val_int_ty((int64_t)*(uint64_t *)p, ITY_U64);
    case SLOT_F32:
        return val_float((double)*(float *)p);
    case SLOT_F64:
        return val_float(*(double *)p);
    case SLOT_BOOL:
        return val_bool(*(uint8_t *)p != 0);
    case SLOT_CHAR:
        return val_char((int64_t)*(uint8_t *)p);
    case SLOT_STR: {
        const char *s = *(const char **)p;
        return val_str(s ? s : "");
    }
    case SLOT_PTR:
        return interp_ptr_value(I, *(void **)p, ts);
    case SLOT_STRUCT:
        return struct_load(I, p, sd);
    default:
        return box_read(I, *(uint64_t *)p);
    }
}

void interp_mem_store(interp_t *I, void *addr, const char *ts, value_t v)
{
    ast_node_t *sd = NULL;
    unsigned char *p = (unsigned char *)addr;
    switch (slot_kind(I, ts, &sd)) {
    case SLOT_I8:
    case SLOT_U8:
    case SLOT_CHAR:
        *(uint8_t *)p = (uint8_t)to_int(v);
        return;
    case SLOT_I16:
    case SLOT_U16:
        *(uint16_t *)p = (uint16_t)to_int(v);
        return;
    case SLOT_I32:
    case SLOT_U32:
        *(uint32_t *)p = (uint32_t)to_int(v);
        return;
    case SLOT_I64:
    case SLOT_U64:
        *(uint64_t *)p = (uint64_t)to_int(v);
        return;
    case SLOT_F32:
        *(float *)p = (float)to_float(v);
        return;
    case SLOT_F64:
        *(double *)p = to_float(v);
        return;
    case SLOT_BOOL:
        *(uint8_t *)p = to_bool(v) ? 1 : 0;
        return;
    case SLOT_STR:
        *(const char **)p = v.kind == VAL_STR ? v.as.s : "";
        return;
    case SLOT_PTR:
        *(void **)p = v.kind == VAL_PTR ? v.as.ptr.addr : (void *)(intptr_t)to_int(v);
        return;
    case SLOT_STRUCT:
        struct_store(I, p, sd, v);
        return;
    default:
        *(uint64_t *)p = box_write(I, v);
        return;
    }
}

/*
 * Count the trailing '*'s of a type string. `Tag*` points at a struct the
 * walker can lay out; `Tag**` points at a pointer, which is one opaque word.
 */
static int star_count(const char *ts)
{
    size_t n = strlen(ts);
    int stars = 0;
    while (n > 0 && ts[n - 1] == '*') {
        stars++;
        n--;
    }
    return stars;
}

sptr_t interp_ptr_from_typestr(interp_t *I, void *addr, const char *ts)
{
    char pointee[96];
    size_t n;
    sptr_t p;
    p.addr = addr;
    p.elem = ptr_elem_from_typestr(ts);
    p.tname = NULL;
    if (p.elem != PTR_OPAQUE || !ts || star_count(ts) != 1) return p;
    n = strlen(ts) - 1;
    while (n > 0 && ts[n - 1] == ' ')
        n--;
    if (n == 0 || n >= sizeof pointee) return p;
    memcpy(pointee, ts, n);
    pointee[n] = 0;
    if (find_struct(I, pointee)) {
        p.tname = arena_strdup(I->a, pointee);
        p.elem = PTR_STRUCT;
    }
    return p;
}

value_t interp_ptr_value(interp_t *I, void *addr, const char *ts)
{
    value_t v;
    v.kind = VAL_PTR;
    v.ity = ITY_NONE;
    v.as.ptr = interp_ptr_from_typestr(I, addr, ts);
    return v;
}

void *interp_ptr_elem_addr(interp_t *I, sptr_t p, int64_t idx)
{
    int64_t stride = p.elem == PTR_STRUCT ? interp_sizeof_typename(I, p.tname)
                                          : (int64_t)ptr_elem_size(p.elem);
    return (unsigned char *)p.addr + idx * stride;
}

static bool field_place(interp_t *I, unsigned char *base, ast_node_t *sd,
                        const char *name, void **addr, const char **ts)
{
    int64_t off = 0;
    size_t i = 0;
    for (; i < sd->list.len; i++) {
        ast_node_t *f = (ast_node_t *)sd->list.data[i];
        const char *fts;
        int64_t sz;
        if (f->kind != AST_FIELD) continue;
        fts = field_type_str(f);
        sz = field_slot(I, fts, &off);
        if (f->name && strcmp(f->name, name) == 0) {
            *addr = base + off;
            *ts = fts;
            return true;
        }
        off += sz;
    }
    return false;
}

/*
 * True when `n` names a place inside real memory: `p[i]` through a pointer to
 * a struct, and any field chain below it. Assigning to one of those has to
 * reach the bytes, because the value a load produced is a copy - a write to
 * `v.ref(i)[0].name` would otherwise land in the copy and be lost with no
 * diagnostic.
 *
 * Returning false costs the caller one repeated evaluation of the indexed
 * expression, which is why only a chain rooted in an index is ever offered
 * here (see interp_resolve_loc).
 */
bool interp_mem_lvalue(interp_t *I, env_t *env, ast_node_t *n, void **addr,
                       const char **ts)
{
    if (n->kind == AST_INDEX) {
        value_t base = eval(I, env, n->a);
        if (base.kind != VAL_PTR || base.as.ptr.elem != PTR_STRUCT || !base.as.ptr.addr)
            return false;
        *addr = interp_ptr_elem_addr(I, base.as.ptr, to_int(eval(I, env, n->b)));
        *ts = base.as.ptr.tname;
        return true;
    }
    if (n->kind == AST_MEMBER) {
        void *base = NULL;
        const char *bts = NULL;
        ast_node_t *sd = NULL;
        if (!interp_mem_lvalue(I, env, n->a, &base, &bts)) return false;
        if (slot_kind(I, bts, &sd) != SLOT_STRUCT) return false;
        return field_place(I, (unsigned char *)base, sd, n->name, addr, ts);
    }
    return false;
}
