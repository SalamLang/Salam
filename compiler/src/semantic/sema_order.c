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
#include "semantic/sema_internal.h"
#include "langpack/langpack.h"

/*
 * Top-level statement ordering:
 *   import/link  ->  const  ->  var  ->  type/struct/enum/interface/impl  ->  func
 * Sections may be empty, but once a later section starts, nothing from an
 * earlier section may appear again. Phase classification lives in
 * ast_toplevel_order_phase() (ast/ast.c) so the formatter's order-fixing
 * pass can reuse the exact same rules.
 */
typedef ast_order_phase_t order_phase_t;
#define ORDER_IMPORT AST_ORDER_IMPORT
#define ORDER_CONST AST_ORDER_CONST
#define ORDER_VAR AST_ORDER_VAR
#define ORDER_TYPEDEF AST_ORDER_TYPEDEF
#define ORDER_FUNC AST_ORDER_FUNC
#define ORDER_NONE AST_ORDER_NONE
#define order_phase_of ast_toplevel_order_phase

static const char *safe_name(const ast_node_t *d)
{
    return (d && d->name) ? d->name : "?";
}

static void check_pub_order(sema_t *s, vec_t *list)
{
    const char *entry = langpack_entry_for(s->lang);
    ast_node_t *first_pub = NULL;
    size_t i = 0;
    for (; i < list->len; i++) {
        ast_node_t *d = (ast_node_t *)list->data[i];
        if (!d || d->synthetic || d->kind != AST_FUNC_DEF) continue;
        if (d->is_extern && !d->a) continue; /* body-less prototype, not a real def */
        /* The program's entry point is conventionally placed last regardless
         * of visibility and isn't part of the file's public API surface, so
         * it's exempt from the pub-ordering rule. */
        if (d->name && (strcmp(d->name, "main") == 0 ||
                        (entry && strcmp(d->name, entry) == 0)))
            continue;
        if (d->is_pub) {
            if (!first_pub) first_pub = d;
        } else if (first_pub) {
            SERR(s, 88, &d->span,
                 "function '%s' must appear before 'pub' function '%s' (line %u): "
                 "once a 'pub' function starts the public section, only other 'pub' "
                 "functions may follow it, so callers can scan the end of the file "
                 "for the public API",
                 safe_name(d), safe_name(first_pub), first_pub->span.begin.line);
        }
    }
}

void sema_check_toplevel_order(sema_t *s, ast_node_t *program)
{
    order_phase_t max_phase = ORDER_IMPORT;
    ast_node_t *first_of[5];
    bool have_first[5];
    memset(first_of, 0, sizeof(first_of));
    memset(have_first, 0, sizeof(have_first));

    size_t i = 0;
    for (; i < program->list.len; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (!d || d->synthetic) continue;
        order_phase_t p = order_phase_of(d);
        if (p == ORDER_NONE) continue;

        if (!have_first[p]) {
            have_first[p] = true;
            first_of[p] = d;
        }

        if (p < max_phase) {
            ast_node_t *blocker = NULL;
            int k = (int)p + 1;
            for (; k <= (int)max_phase; k++) {
                if (have_first[k]) {
                    blocker = first_of[k];
                    break;
                }
            }
            if (!blocker) blocker = d;

            switch (p) {
            case ORDER_IMPORT:
                SERR(s, 83, &d->span,
                     "'import' must appear before any other top-level statement, "
                     "right after the package declaration; '%s' (line %u) already "
                     "starts the rest of the file",
                     safe_name(blocker), blocker->span.begin.line);
                break;
            case ORDER_CONST:
                if (max_phase >= ORDER_TYPEDEF)
                    SERR(s, 84, &d->span,
                         "constant '%s' must be declared before any function or "
                         "type definition; move it up near the top of the file "
                         "with the other constants, above '%s' (line %u)",
                         safe_name(d), safe_name(blocker), blocker->span.begin.line);
                else
                    SERR(s, 86, &d->span,
                         "constant '%s' must be declared before global variables; "
                         "move it above the variable section, before '%s' (line %u)",
                         safe_name(d), safe_name(blocker), blocker->span.begin.line);
                break;
            case ORDER_VAR:
                SERR(s, 85, &d->span,
                     "global variable '%s' must be declared before any function or "
                     "type definition; move it up near the top of the file, above "
                     "'%s' (line %u)",
                     safe_name(d), safe_name(blocker), blocker->span.begin.line);
                break;
            case ORDER_TYPEDEF:
                SERR(s, 87, &d->span,
                     "'%s' must be declared before any function definition; keep "
                     "type, struct, enum, interface and impl declarations grouped "
                     "above the functions, above '%s' (line %u)",
                     safe_name(d), safe_name(blocker), blocker->span.begin.line);
                break;
            default:
                break;
            }
        } else if (p > max_phase) {
            max_phase = p;
        }
    }

    check_pub_order(s, &program->list);
    {
        size_t j = 0;
        for (; j < program->list.len; j++) {
            ast_node_t *d = (ast_node_t *)program->list.data[j];
            if (d && !d->synthetic && d->kind == AST_IMPL_DEF)
                check_pub_order(s, &d->list);
        }
    }
}

void sema_check_toplevel_order_in_file(sema_t *s, ast_node_t *program, const char *file)
{
    const char *save_file = s->file;
    s->file = file;
    sema_check_toplevel_order(s, program);
    s->file = save_file;
}
