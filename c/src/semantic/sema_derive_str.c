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
 * `println p` on a struct, an array, a Vector or a HashMap.
 *
 * printf has no spelling for an aggregate, so the value is turned into a
 * string first: this pass writes a `func _strof_T(_v: T): str` that appends
 * every field or element to a buffer, installs it in the program being
 * analysed, and rewrites the print argument into a call to it. A field that
 * is itself an aggregate gets its own function, derived on the way through,
 * so a Vector of structs nests without any of this knowing how deep it goes.
 *
 * The function is generated as Salam source and put through the lexer and
 * parser, rather than assembled as AST by hand. Two reasons: what a printed
 * value looks like is then readable in one place instead of spread over node
 * constructors, and the result is an ordinary synthetic top-level
 * declaration - exactly what a generic instantiation is - so the C, LLVM, JS
 * and interpreter backends emit it without knowing this pass exists.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "core/sb.h"
#include "semantic/sema_derive_core.h"
#include "semantic/sema_derive_internal.h"

const char *sema_derive_stringify(sema_t *s, type_t *t, const src_span_t *span)
{
    const char *fname;
    derived_t *ent;
    const char *text;
    ast_node_t *fn;

    if (!t || !s->program) return NULL;
    fname = derive_fn_name(s, "_strof_", t);
    if (derive_claimed(s, fname)) return fname;
    if (scope_lookup_local(s->global, fname)) return NULL;

    ent = derive_claim(s, fname);
    text = derive_fn_source(s, t, fname, span);
    if (!text) {
        derive_unclaim(s, ent);
        return NULL;
    }
    fn = derive_install_source(s, text, fname, NULL);
    if (!fn) {
        derive_unclaim(s, ent);
        return NULL;
    }
    derive_set_decl(ent, fn);
    return fname;
}
