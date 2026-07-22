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
#include "core/sal_format.h"
#include "semantic/sema_internal.h"

static type_t *ty(sema_t *s, type_kind_t k)
{
    return sema_ty(s, k);
}

static type_t *err_ty(sema_t *s)
{
    return sema_err_ty(s);
}

static type_t *decorate(sema_t *s, ast_node_t *n, type_t *t)
{
    return sema_decorate(s, n, t);
}

/* If a block arm's last statement is a bare expression (no explicit 'ret'),
 * rewrite it in place into a synthetic AST_RETURN so every arm's
 * value-producing statement is uniformly an AST_RETURN by the time it's
 * checked -- mirrors the synthetic-return wrapping parse_match_arm already
 * does at parse time for single-expression arms. */
static void rewrite_tail_expr(sema_t *s, ast_node_t *block)
{
    ast_node_t *last;
    ast_node_t *ret;
    if (!block || block->list.len == 0) return;
    last = (ast_node_t *)block->list.data[block->list.len - 1];
    if (last->kind != AST_EXPR_STMT) return;
    ret = ast_new(s->a, AST_RETURN, &last->span);
    ret->synthetic = true;
    ret->a = last->a;
    block->list.data[block->list.len - 1] = ret;
}

static bool types_comparable(sema_t *s, type_t *a, type_t *b)
{
    if (type_is_error(a) || type_is_error(b)) return true;
    if (type_equiv(a, b)) return true;
    return type_is_numeric(a) && type_is_numeric(b) &&
           type_common_arith(s->tc, a, b) != NULL;
}

/* Checks one arm pattern against the subject's type. Returns the resolved
 * Variant member tag (>= 0) when `subj` is a Variant and the pattern names
 * one of its member types; returns -1 for the ordinary value-pattern path
 * (or when an error was already reported). A bare identifier value pattern
 * matched against an enum subject is rewritten in place into a qualified
 * `Enum.member` AST_MEMBER node so every backend's existing enum-member
 * codegen handles it unchanged -- no new backend code needed for it. */
static int check_match_pattern(sema_t *s, ast_node_t *pat, type_t *subj,
                               bool subj_is_variant, bool subj_is_enum, type_t **pt_out)
{
    if (subj_is_variant) {
        type_t *pt;
        int tag;
        if (pat->a->kind != AST_IDENTIFIER) {
            SERR(s, 76, &pat->span,
                 "expected a type-name pattern to match a Variant subject");
            return -1;
        }
        pt = sema_resolve_type(s, pat->a);
        if (type_is_error(pt)) return -1;
        tag = type_variant_tag(subj, pt);
        if (tag < 0)
            SERR(s, 75, &pat->span, "match arm pattern type '%s' is not a member of '%s'",
                 type_to_string(s->tc, pt), type_to_string(s->tc, subj));
        pat->type_str = type_to_string(s->tc, pt);
        pat->value.kind = TV_INT;
        pat->value.as.i = (uint64_t)(tag < 0 ? 0 : tag);
        if (pt_out) *pt_out = pt;
        return tag;
    }

    if (pat->name)
        SERR(s, 77, &pat->span,
             "value pattern used on a non-Variant match subject of type '%s'; a "
             "type-name pattern (with an optional binding) is only valid when "
             "matching a Variant",
             type_to_string(s->tc, subj));

    if (pat->a->kind == AST_LITERAL) {
        type_t *pt;
        s->expected = subj;
        pt = sema_check_expr(s, pat->a);
        s->expected = NULL;
        if (!types_comparable(s, subj, pt))
            SERR(s, 21, &pat->span, "cannot compare '%s' and '%s'",
                 type_to_string(s->tc, subj), type_to_string(s->tc, pt));
    } else if (pat->a->kind == AST_IDENTIFIER) {
        if (subj_is_enum) {
            symbol_t *sy = (symbol_t *)subj->decl;
            symbol_t *m = (sy && sy->members)
                              ? scope_lookup_local(sy->members, pat->a->name)
                              : NULL;
            if (!m) {
                SERR(s, 16, &pat->a->span, "enum '%s' has no member '%s'",
                     sy ? sy->name : "?", pat->a->name);
            } else {
                ast_node_t *mem = ast_new(s->a, AST_MEMBER, &pat->a->span);
                mem->synthetic = true;
                mem->a = ast_new(s->a, AST_IDENTIFIER, &pat->a->span);
                mem->a->synthetic = true;
                mem->a->name = sy->name;
                mem->name = pat->a->name;
                pat->a = mem;
                sema_check_expr(s, pat->a);
            }
        } else {
            type_t *pt = sema_check_expr(s, pat->a);
            if (!types_comparable(s, subj, pt))
                SERR(s, 21, &pat->span, "cannot compare '%s' and '%s'",
                     type_to_string(s->tc, subj), type_to_string(s->tc, pt));
        }
    }
    return -1;
}

type_t *sema_check_match(sema_t *s, ast_node_t *n, bool is_expr, type_t *expected)
{
    type_t *exp = expected;
    type_t *subj;
    bool subj_is_variant, subj_is_enum, has_wildcard = false;
    vec_t arm_types;
    vec_t arm_yields;
    vec_t seen_members;
    vec_t seen_tags;
    size_t ai;

    s->expected = NULL;
    subj = sema_check_expr(s, n->a);
    n->match_is_expr = is_expr;

    subj_is_variant = subj && subj->kind == TY_VARIANT;
    subj_is_enum = subj && subj->kind == TY_ENUM;

    vec_init(&arm_types);
    vec_init(&arm_yields);
    vec_init(&seen_members);
    vec_init(&seen_tags);

    ai = 0;
    for (; ai < n->list.len; ai++) {
        ast_node_t *arm = (ast_node_t *)n->list.data[ai];
        type_t *arm_ty = NULL;
        vec_t *collect = NULL;

        if (arm->op == TK_KW_ELSE) {
            has_wildcard = true;
        } else {
            size_t pi = 0;
            for (; pi < arm->list.len; pi++) {
                ast_node_t *pat = (ast_node_t *)arm->list.data[pi];
                int tag = check_match_pattern(s, pat, subj, subj_is_variant, subj_is_enum,
                                              NULL);
                if (subj_is_variant) {
                    if (tag >= 0) {
                        size_t k = 0;
                        bool dup = false;
                        for (; k < seen_tags.len; k++)
                            if ((size_t)seen_tags.data[k] == (size_t)tag) {
                                dup = true;
                                break;
                            }
                        if (dup)
                            SERR(s, 78, &pat->span, "duplicate match arm pattern");
                        else
                            vec_push(s->a, &seen_tags, (void *)(size_t)tag);
                        if (!dup && pat->name && arm->list.len == 1) {
                            ast_node_t *decl = ast_new(s->a, AST_VAR_DECL, &pat->span);
                            ast_node_t *unwrap =
                                ast_new(s->a, AST_VARIANT_UNWRAP, &pat->span);
                            decl->synthetic = true;
                            decl->name = pat->name;
                            unwrap->synthetic = true;
                            unwrap->a = ast_clone(s->a, n->a);
                            unwrap->type = pat->a;
                            decl->a = unwrap;
                            {
                                vec_t newlist;
                                size_t k2 = 0;
                                vec_init(&newlist);
                                vec_push(s->a, &newlist, decl);
                                for (; k2 < arm->b->list.len; k2++)
                                    vec_push(s->a, &newlist, arm->b->list.data[k2]);
                                arm->b->list = newlist;
                            }
                        }
                    }
                } else if (subj_is_enum && pat->a->kind == AST_MEMBER) {
                    symbol_t *sy = (symbol_t *)subj->decl;
                    symbol_t *m = (sy && sy->members)
                                      ? scope_lookup_local(sy->members, pat->a->name)
                                      : NULL;
                    if (m) {
                        size_t k = 0;
                        bool dup = false;
                        for (; k < seen_members.len; k++)
                            if (seen_members.data[k] == (void *)m) {
                                dup = true;
                                break;
                            }
                        if (dup)
                            SERR(s, 78, &pat->span, "duplicate match arm pattern '%s'",
                                 pat->a->name);
                        else
                            vec_push(s->a, &seen_members, m);
                    }
                }
            }
            if (arm->list.len > 1) {
                size_t pi2 = 0;
                for (; pi2 < arm->list.len; pi2++) {
                    ast_node_t *pat = (ast_node_t *)arm->list.data[pi2];
                    if (pat->name)
                        SERR(s, 76, &pat->span,
                             "a match arm with multiple patterns cannot bind a name; use "
                             "a single-pattern arm to bind, e.g. 'i32 n =>'");
                }
            }
        }

        if (is_expr) rewrite_tail_expr(s, arm->b);

        if (is_expr) {
            int saved_depth = s->match_arm_depth;
            type_t *saved_yexp = s->match_yield_expected;
            vec_t *saved_collect = s->match_yield_collect;
            collect = (vec_t *)arena_alloc(s->a, sizeof(vec_t));
            vec_init(collect);
            s->match_arm_depth = saved_depth + 1;
            s->match_yield_expected = exp;
            s->match_yield_collect = collect;
            sema_check_block(s, arm->b);
            s->match_arm_depth = saved_depth;
            s->match_yield_expected = saved_yexp;
            s->match_yield_collect = saved_collect;

            if (collect->len == 0) {
                SERR(s, 81, &arm->span,
                     "'match' used as an expression must have arms that all produce a "
                     "value");
                arm_ty = err_ty(s);
            } else {
                size_t yi = 1;
                match_yield_t *y0 = (match_yield_t *)collect->data[0];
                type_t *t0 = y0->type;
                bool ok = true;
                for (; yi < collect->len; yi++) {
                    match_yield_t *y = (match_yield_t *)collect->data[yi];
                    if (type_equiv(t0, y->type)) continue;
                    if (type_is_numeric(t0) && type_is_numeric(y->type)) {
                        type_t *c = type_common_arith(s->tc, t0, y->type);
                        if (c) {
                            t0 = c;
                            continue;
                        }
                    }
                    ok = false;
                }
                if (!ok) {
                    SERR(s, 81, &arm->span,
                         "match arm produces different types on different code paths");
                    arm_ty = err_ty(s);
                } else {
                    arm_ty = t0;
                }
            }
        } else {
            sema_check_block(s, arm->b);
        }

        vec_push(s->a, &arm_types, arm_ty);
        vec_push(s->a, &arm_yields, collect);
    }

    if (!has_wildcard && subj_is_enum && subj->decl) {
        symbol_t *sy = (symbol_t *)subj->decl;
        size_t total = sy->members ? sy->members->symbols.len : 0;
        if (seen_members.len < total) {
            char buf[256];
            size_t o = 0;
            size_t mi = 0;
            for (; mi < sy->members->symbols.len && o < sizeof(buf) - 32; mi++) {
                symbol_t *m = (symbol_t *)sy->members->symbols.data[mi];
                size_t k = 0;
                bool covered = false;
                for (; k < seen_members.len; k++)
                    if (seen_members.data[k] == (void *)m) {
                        covered = true;
                        break;
                    }
                if (!covered)
                    o = sal_catf(buf, sizeof(buf), o, "%s%s", o ? ", " : "", m->name);
            }
            if (mi < sy->members->symbols.len) o = sal_catf(buf, sizeof(buf), o, ", ...");
            SWARN(s, 80, &n->span,
                  "non-exhaustive match on enum '%s': missing member(s) %s", sy->name,
                  buf);
        }
    }
    if (!has_wildcard && subj_is_variant) {
        size_t total = subj->params.len;
        if (seen_tags.len < total) {
            char buf[256];
            size_t o = 0;
            size_t ti = 0;
            for (; ti < total && o < sizeof(buf) - 32; ti++) {
                size_t k = 0;
                bool covered = false;
                for (; k < seen_tags.len; k++)
                    if ((size_t)seen_tags.data[k] == ti) {
                        covered = true;
                        break;
                    }
                if (!covered)
                    o = sal_catf(buf, sizeof(buf), o, "%s%s", o ? ", " : "",
                                 type_to_string(s->tc, (type_t *)subj->params.data[ti]));
            }
            if (ti < total) o = sal_catf(buf, sizeof(buf), o, ", ...");
            SERR(s, 79, &n->span,
                 "non-exhaustive match on Variant '%s': missing case(s) for %s",
                 type_to_string(s->tc, subj), buf);
        }
    }

    if (!is_expr) return decorate(s, n, ty(s, TY_VOID));

    {
        type_t *first = NULL;
        bool all_equiv = true;
        type_t *unified;
        size_t k;

        k = 0;
        for (; k < arm_types.len; k++) {
            type_t *t = (type_t *)arm_types.data[k];
            if (!t || type_is_error(t)) continue;
            if (!first) {
                first = t;
                continue;
            }
            if (!type_equiv(first, t)) all_equiv = false;
        }

        if (!first) {
            unified = err_ty(s);
        } else if (all_equiv) {
            unified = first;
        } else {
            type_t *acc = NULL;
            bool numeric_ok = true;
            k = 0;
            for (; k < arm_types.len; k++) {
                type_t *t = (type_t *)arm_types.data[k];
                if (!t || type_is_error(t)) continue;
                if (!acc) {
                    acc = t;
                    continue;
                }
                {
                    type_t *c = type_common_arith(s->tc, acc, t);
                    if (!c) {
                        numeric_ok = false;
                        break;
                    }
                    acc = c;
                }
            }
            if (numeric_ok && acc) {
                unified = acc;
            } else {
                type_t *members[64];
                size_t nmem = 0;
                k = 0;
                for (; k < arm_types.len; k++) {
                    type_t *t = (type_t *)arm_types.data[k];
                    size_t j;
                    bool dup = false;
                    if (!t || type_is_error(t)) continue;
                    for (j = 0; j < nmem; j++)
                        if (type_equiv(members[j], t)) {
                            dup = true;
                            break;
                        }
                    if (dup) continue;
                    if (nmem >= 64) {
                        SERR(s, 1, &n->span,
                             "match expression has too many distinct arm types (max 64)");
                        break;
                    }
                    members[nmem++] = t;
                }
                unified = type_variant(s->tc, members, nmem);
            }
        }

        if (unified->kind == TY_VARIANT) {
            k = 0;
            for (; k < arm_types.len; k++) {
                type_t *at = (type_t *)arm_types.data[k];
                vec_t *acol = (vec_t *)arm_yields.data[k];
                int tag;
                size_t yi;
                if (!at || type_is_error(at) || !acol) continue;
                tag = type_variant_tag(unified, at);
                if (tag < 0) continue;
                yi = 0;
                for (; yi < acol->len; yi++) {
                    match_yield_t *y = (match_yield_t *)acol->data[yi];
                    y->ret->a = coerce_to_variant(s, unified, y->ret->a, tag);
                }
            }
        }

        n->type_str = type_to_string(s->tc, unified);
        return decorate(s, n, unified);
    }
}
