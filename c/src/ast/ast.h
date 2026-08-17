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

#ifndef SALAM_AST_AST_H
#define SALAM_AST_AST_H

#include "core/prelude.h"
#include "core/arena.h"
#include "core/vec.h"
#include "core/span.h"
#include "token/token.h"
#include "xml/xml.h"

/* Placeholder the parser stores as a declaration's or an identifier's name
 * once it has already reported why the real name could not be read. Later
 * phases must stay silent about it: a second diagnostic quoting "<error>"
 * only buries the one that explained the problem. */
#define SALAM_ERR_NAME "<error>"

SAL_INLINE bool ast_name_is_err(const char *name)
{
    return name && strcmp(name, SALAM_ERR_NAME) == 0;
}

typedef enum {
    AST_PROGRAM = 0,
    AST_IMPORT,
    AST_LINK,
    AST_TYPE_ALIAS,
    AST_CONST_DECL,
    AST_FUNC_DEF,
    AST_PARAM,
    AST_STRUCT_DEF,
    AST_FIELD,
    AST_INTERFACE_DEF,
    AST_IMPL_DEF,
    AST_ENUM_DEF,
    AST_ENUM_MEMBER,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_IF,
    AST_UNTIL,
    AST_FOR,
    AST_REPEAT,
    AST_EACH,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_DEFER,
    AST_EXPR_STMT,
    AST_BINARY,
    AST_UNARY,
    AST_INCDEC,
    AST_CAST,
    AST_LITERAL,
    AST_IDENTIFIER,
    AST_FUNC_ADDR,
    AST_THIS,
    AST_CALL,
    AST_MEMBER,
    AST_INDEX,
    AST_SLICE,
    AST_TERNARY,
    AST_LAMBDA,
    AST_ARRAY_LIT,
    AST_STRUCT_LIT,
    AST_FIELD_INIT,
    AST_TYPE,
    AST_LAYOUT_BLOCK,
    AST_LAYOUT_ELEMENT,
    AST_LAYOUT_ATTR,
    AST_LAYOUT_COMPONENT,
    AST_MATCH,
    AST_MATCH_ARM,
    AST_MATCH_PATTERN,
    AST_VARIANT_BOX,
    AST_VARIANT_UNWRAP,
    AST__COUNT
} ast_kind_t;

typedef struct ast_node ast_node_t;

struct ast_node {
    ast_kind_t kind;
    src_span_t span;
    /* Source file this top-level node was parsed from, stamped by parser_run.
     * The files of a multi-file package are merged into one program, so the
     * span alone cannot say which file a line number belongs to; sema follows
     * this while checking so a sibling file's diagnostics stop being reported
     * against the package's anchor file. NULL on nodes below the top level and
     * on synthetic ones - those inherit whatever file is current. */
    const char *src_file;
    const char *name;
    token_value_t value;
    token_kind_t op;
    bool is_mut;
    bool is_pointer;
    bool synthetic;
    bool file_boundary; /* first top-level node merged in from a package file;
                         * resets per-file top-level ordering checks */
    bool is_extern;
    bool is_variadic;
    bool is_dyn;
    bool is_ref;
    /* Set on a call argument that binds to a `&:` reference parameter, so the
     * definite-assignment pass can tell "the callee writes this" apart from
     * "this value is read". Stamped by sema_call.c's mark_ref_args(). */
    bool ref_arg;
    bool is_pub;
    /* Set on an AST_IDENTIFIER that names a free function and is being read as
     * a value rather than called. Sema types it i64 (the handler-slot ABI that
     * std/net/router and friends take) and every backend lowers it to the
     * function's address. */
    bool func_value;
    bool is_inline;
    bool is_noinline;
    bool is_pure;
    bool is_noret;
    bool is_deprecated;
    bool is_slice;
    bool is_prefix;
    bool match_is_expr;
    /* Set on the synthetic AST_VAR_DECLs an `each` lowers its key/value
     * bindings to, so the unused-variable check can tell a loop binding from
     * an ordinary local and apply the stricter rule. */
    bool loop_bind;
    ast_node_t *type;
    ast_node_t *a, *b, *c, *d;
    vec_t list;
    vec_t dims;
    vec_t typarams;
    vec_t typaram_bounds;
    vec_t aliases;
    vec_t captures;
    const char *type_str;
    /*
     * On an AST_TYPE the generic machinery built rather than parsed: the
     * type_t it already stands for. A type argument is resolved in the
     * caller's scope but substituted into a template that is checked in the
     * declaring package's scope, and a package's scope cannot see the
     * caller's declarations - so a round trip through the name would lose a
     * user-defined struct handed to a stdlib generic ("unknown type 'User'"
     * from inside std/encoding/json). Carrying the resolved type across that
     * switch is what keeps `json.Marshal(user)` and `option.Some(user)`
     * working. NULL on everything the parser produced. Void so ast.h stays
     * free of the semantic layer; sema_resolve_type is the only reader.
     */
    void *sema_type;
    /*
     * The same trick one level down, for an occurrence that decorates the type
     * parameter rather than naming it bare: `T*`, `T[4]`. Substituting `T` =
     * `Row` there cannot reuse the argument's resolved type - the node stands
     * for `Row*`, not `Row` - but resolving the name again is exactly what
     * fails, because `Row` belongs to a package the template's scope cannot
     * see. So the *base* travels instead and sema_resolve_type applies this
     * node's own `*` and `[N]` on top of it. NULL unless g_subst set it.
     */
    void *sema_base_type;
    /*
     * On a synthetic top-level function - a generic instantiation or a derived
     * codec - the package whose scope its body was checked inside. Its name is
     * global, but the names it calls are that package's, private ones
     * included. The interpreter keeps one environment per module, so without
     * this a body would be looked up against the program's globals and its own
     * module's helpers would be missing ("call to undefined function
     * '_dnew'"). NULL for everything else.
     */
    const char *home_pkg;
    const char *origin_lang;
    /* Interpreter-only inline cache for identifier lookups; ignored by every
     * other consumer of the AST. Resolving a name meant walking the scope
     * chain and strcmp-ing at each level, on every single evaluation. Lexical
     * scoping means an identifier node resolves to the same relative place
     * every time, so the interpreter records where it landed (`ic_hops`
     * parents up, slot `ic_slot`) and revalidates with one strcmp instead of a
     * search. See interp_expr.c's AST_IDENTIFIER case. */
    uint32_t ic_hops;
    uint32_t ic_slot;
    bool ic_valid;
};

ast_node_t *ast_new(arena_t *a, ast_kind_t kind, const src_span_t *span);

/* Total nodes allocated by ast_new() since process start. Reported by
 * --time-report; not reset between builds. */
uint64_t ast_node_count(void);

void ast_add(arena_t *a, ast_node_t *parent, ast_node_t *child);

const char *ast_kind_name(ast_kind_t kind);

typedef enum {
    AST_ORDER_IMPORT = 0,
    AST_ORDER_CONST = 1,
    AST_ORDER_VAR = 2,
    AST_ORDER_TYPEDEF = 3,
    AST_ORDER_FUNC = 4,
    AST_ORDER_NONE = -1
} ast_order_phase_t;

ast_order_phase_t ast_toplevel_order_phase(const ast_node_t *d);

long ast_str_lit_len(const ast_node_t *n);

ast_node_t *ast_clone(arena_t *a, const ast_node_t *n);

void ast_to_xml(xml_writer_t *w, const ast_node_t *node);

#endif /* SALAM_AST_AST_H */
