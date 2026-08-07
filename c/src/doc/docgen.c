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
 * `salam doc` engine: lexes + parses (+ a best-effort semantic pass, for real
 * inferred types on ':='-declared consts/vars) one file at a time, walks the
 * resulting AST into a flat "DocDb", then renders the whole database as one
 * self-contained HTML file (inline CSS + JS, no external assets). No codegen,
 * no build - this module never touches codegen/jsgen/llvm.
 *
 * This is the C port of the self-hosted compiler/docgen.salam; the two are
 * expected to produce byte-identical HTML for the same inputs. The static
 * assets are shared verbatim through the generated doc_assets.c (see
 * c/tools/python/gen_doc_assets.py).
 *
 * Doc comments are a Salam convention, not a language feature: a contiguous
 * run of leading '//'/'///' line comments, or a single block comment, with no
 * blank line between it and the declaration it precedes, becomes that
 * declaration's doc text. Optional '@lang' tags split it into per-language
 * prose, reusing the tagging idea of the existing '@en "..."'/'@fa "..."'
 * name-alias metas (parse_metas() in parser.c).
 *
 * Because the parser drops comments, doc text comes from a second,
 * comment-preserving lex of the same source (lexer_run_ex with keep_comments);
 * that token stream is never fed to the parser.
 */

#include "core/prelude.h"
#include "core/sal_format.h"
#include "doc/docgen.h"
#include "ast/ast.h"
#include "token/token.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "source/source.h"
#include "semantic/sema.h"
#include "condcomp/condcomp.h"

#define DOC_FILE_ARENA (1 << 20)

/* ==========================================================================
 * Small text helpers
 *
 * Trimming must match the Salam stdlib's str.Trim exactly (std/text/text.salam's
 * _is_ws), which strips these four ASCII bytes and nothing else - notably not
 * vertical tab, form feed, or any Unicode space.
 * ========================================================================== */

static bool doc_is_space(char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool doc_is_alpha(char c)
{
    unsigned char u = (unsigned char)c;
    return (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z');
}

/* Narrows [s, s+n) to its trimmed span. Aliasing `s` with `*out` is safe: the
 * arguments are evaluated before the call. */
static void trim_span(const char *s, size_t n, const char **out, size_t *outn)
{
    size_t b = 0, e = n;
    while (b < e && doc_is_space(s[b]))
        b++;
    while (e > b && doc_is_space(s[e - 1]))
        e--;
    *out = s + b;
    *outn = e - b;
}

static void sb_put_span(sb_t *s, const char *p, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
        sb_putc(s, p[i]);
}

/*
 * Line splitter matching docgen.salam's split_lines: splits on '\n' and always
 * yields a final (possibly empty) segment after the last one, so "a\n" is two
 * lines and "" is one empty line.
 */
typedef struct {
    const char *s;
    size_t len;
    size_t pos;
    bool done;
} line_iter_t;

static void li_init(line_iter_t *it, const char *s, size_t len)
{
    it->s = s;
    it->len = len;
    it->pos = 0;
    it->done = false;
}

static bool li_next(line_iter_t *it, const char **out, size_t *outn)
{
    size_t start;
    if (it->done) return false;
    start = it->pos;
    while (it->pos < it->len && it->s[it->pos] != '\n')
        it->pos++;
    *out = it->s + start;
    *outn = it->pos - start;
    if (it->pos < it->len)
        it->pos++; /* consume the '\n' */
    else
        it->done = true;
    return true;
}

/* ==========================================================================
 * DocDb row builders
 * ========================================================================== */

static void add_pair_n(arena_t *a, vec_t *v, const char *lang, const char *text,
                       size_t textlen)
{
    doc_pair_t *p = (doc_pair_t *)arena_alloc(a, sizeof *p);
    p->lang = arena_strdup(a, lang ? lang : "");
    p->text = arena_strndup(a, text ? text : "", textlen);
    vec_push(a, v, p);
}

static void add_pair(arena_t *a, vec_t *v, const char *lang, const char *text)
{
    add_pair_n(a, v, lang, text, text ? strlen(text) : 0);
}

static int db_add_file(doc_db_t *db, const char *rel)
{
    int idx = (int)db->files.len;
    vec_push(db->arena, &db->files, CONST_CAST(arena_strdup(db->arena, rel)));
    return idx;
}

/*
 * Every string handed to the db is copied into the db's own arena: the caller
 * parses each file into a short-lived arena that is released as soon as that
 * file's walk finishes.
 */
static doc_decl_t *db_add_decl(doc_db_t *db, doc_kind_t kind, const char *name,
                               const char *sig, bool is_pub, bool is_dep, int file_idx,
                               int line, doc_decl_t *parent)
{
    doc_decl_t *d = (doc_decl_t *)arena_alloc(db->arena, sizeof *d);
    memset(d, 0, sizeof *d);
    d->kind = kind;
    d->name = arena_strdup(db->arena, name ? name : "");
    d->sig = arena_strdup(db->arena, sig ? sig : "");
    d->is_pub = is_pub;
    d->is_deprecated = is_dep;
    d->file_idx = file_idx;
    d->line = line;
    d->self = (int)db->decls.len;
    d->parent = parent ? parent->self : -1;
    vec_init(&d->members);
    vec_init(&d->aliases);
    vec_init(&d->doc);
    vec_push(db->arena, &db->decls, d);
    if (parent) vec_push(db->arena, &parent->members, d);
    return d;
}

/* Node aliases are stored flat as [lang, name, lang, name, ...]; see
 * parse_metas() in parser.c. */
static void attach_aliases(doc_db_t *db, const ast_node_t *n, doc_decl_t *d)
{
    size_t i;
    for (i = 0; i + 1 < n->aliases.len; i += 2)
        add_pair(db->arena, &d->aliases, (const char *)n->aliases.data[i],
                 (const char *)n->aliases.data[i + 1]);
}

/* ==========================================================================
 * Type and signature rendering (purely syntactic - walks AST_TYPE trees)
 * ========================================================================== */

/* finish_untyped_params() in parser_decl.c synthesizes "__T0"-style type-param
 * names for untyped params; those are an implementation detail, never shown. */
static bool is_synth_typaram(const char *name)
{
    return name != NULL && strncmp(name, "__T", 3) == 0;
}

static void type_to_sb(sb_t *out, const ast_node_t *t)
{
    size_t i;

    if (!t) return;

    if (t->is_dyn) {
        sb_puts(out, "dyn ");
        sb_puts(out, t->name ? t->name : "");
    } else if (t->name && strcmp(t->name, "func") == 0) {
        /* A function type keeps its parameter types in `list`, so the generic
         * argument branch below must not also run for it. */
        sb_puts(out, "func(");
        for (i = 0; i < t->list.len; i++) {
            if (i) sb_puts(out, ", ");
            type_to_sb(out, (const ast_node_t *)t->list.data[i]);
        }
        sb_putc(out, ')');
        if (t->type) {
            sb_puts(out, ": ");
            type_to_sb(out, t->type);
        }
    } else {
        sb_puts(out, is_synth_typaram(t->name) ? "auto" : (t->name ? t->name : ""));
        if (t->list.len > 0) {
            sb_putc(out, '<');
            for (i = 0; i < t->list.len; i++) {
                if (i) sb_puts(out, ", ");
                type_to_sb(out, (const ast_node_t *)t->list.data[i]);
            }
            sb_putc(out, '>');
        }
    }

    for (i = 0; i < t->dims.len; i++)
        sb_puts(out, "[]");
    if (t->is_pointer) sb_putc(out, '*');
}

/*
 * Both front-end passes fall back to bracketed placeholders when they give up:
 * the parser substitutes "<error>" for a name it could not read (p_name in
 * parser_cursor.c), and the semantic pass stringifies an unresolved type as
 * "<error>" or "<null>" (type_to_string in types.c). Neither tells a reader
 * anything, and unresolved types are routine here - documenting is per-file, so
 * a type declared in a sibling file simply is not in scope. A real type name
 * can never start with '<', which makes the check both simple and future-proof.
 */
static bool is_placeholder(const char *s)
{
    return s == NULL || s[0] == '\0' || s[0] == '<';
}

/* Renders `t` into `out`, but only if it came out as a usable type name.
 * Returns false, leaving `out` untouched, for placeholders and empties. */
static bool render_type(sb_t *out, const ast_node_t *t)
{
    sb_t tmp;
    bool ok;
    sb_init(&tmp);
    type_to_sb(&tmp, t);
    ok = !is_placeholder(sb_cstr(&tmp));
    if (ok) sb_puts(out, sb_cstr(&tmp));
    sb_free(&tmp);
    return ok;
}

/*
 * Best-effort one-line rendering of a simple literal initializer, used only
 * where neither an explicit annotation nor a semantic-resolved type is
 * available. Returns false, writing nothing, for anything that is not a plain
 * literal.
 */
static bool literal_preview(sb_t *out, const ast_node_t *n)
{
    if (!n || n->kind != AST_LITERAL) return false;
    switch (n->value.kind) {
    case TV_INT:
        sb_printf(out, "%lld", (long long)n->value.as.i);
        return true;
    case TV_FLOAT:
        sb_printf(out, "%g", n->value.as.f);
        return true;
    case TV_STRING:
        /* Deliberately unescaped: the whole signature is escaped once, at
         * emit_js_str() time. */
        sb_putc(out, '"');
        sb_puts(out, n->value.as.s ? n->value.as.s : "");
        sb_putc(out, '"');
        return true;
    case TV_BOOL:
        sb_puts(out, n->value.as.b ? "true" : "false");
        return true;
    default:
        return false;
    }
}

/* Renders "<T, U: Bound>", skipping synthesized params. Emits nothing at all
 * when every param is synthesized, so `func f(x)` stays `func f(x)`. */
static void typarams_to_sb(sb_t *out, const ast_node_t *n)
{
    sb_t b;
    size_t i;
    int shown = 0;

    if (n->typarams.len == 0) return;

    sb_init(&b);
    for (i = 0; i < n->typarams.len; i++) {
        const char *tp = (const char *)n->typarams.data[i];
        const char *bound;
        if (!tp || is_synth_typaram(tp)) continue;
        if (shown) sb_puts(&b, ", ");
        sb_puts(&b, tp);
        bound = i < n->typaram_bounds.len ? (const char *)n->typaram_bounds.data[i] : NULL;
        if (bound && bound[0]) {
            sb_puts(&b, ": ");
            sb_puts(&b, bound);
        }
        shown++;
    }
    if (shown) {
        sb_putc(out, '<');
        sb_puts(out, sb_cstr(&b));
        sb_putc(out, '>');
    }
    sb_free(&b);
}

static void params_to_sb(sb_t *out, const ast_node_t *fn)
{
    size_t i;
    int shown = 0;

    sb_putc(out, '(');
    for (i = 0; i < fn->list.len; i++) {
        const ast_node_t *p = (const ast_node_t *)fn->list.data[i];
        if (p->kind != AST_PARAM) continue;
        if (shown) sb_puts(out, ", ");
        sb_puts(out, p->name ? p->name : "");
        if (p->is_ref) sb_puts(out, " &");
        if (p->type) {
            sb_t t;
            sb_init(&t);
            /* "auto" means the parser synthesized the type; show the bare name. */
            if (render_type(&t, p->type) && strcmp(sb_cstr(&t), "auto") != 0) {
                sb_puts(out, ": ");
                sb_puts(out, sb_cstr(&t));
            }
            sb_free(&t);
        }
        if (p->a) {
            sb_t v;
            sb_init(&v);
            if (literal_preview(&v, p->a)) {
                sb_puts(out, " = ");
                sb_puts(out, sb_cstr(&v));
            }
            sb_free(&v);
        }
        shown++;
    }
    sb_putc(out, ')');
}

static void func_sig_to_sb(sb_t *out, const ast_node_t *n)
{
    sb_puts(out, "func ");
    sb_puts(out, n->name ? n->name : "");
    typarams_to_sb(out, n);
    params_to_sb(out, n);
    if (n->type) {
        sb_t t;
        sb_init(&t);
        if (render_type(&t, n->type)) {
            sb_puts(out, ": ");
            sb_puts(out, sb_cstr(&t));
        }
        sb_free(&t);
    }
}

/* Explicit annotation first, else the semantic pass's inferred type_str (which
 * is what covers ':=' declarations), else nothing. */
static void append_type_anno(sb_t *out, const ast_node_t *n)
{
    sb_t t;
    sb_init(&t);
    if (n->type)
        render_type(&t, n->type);
    else if (!is_placeholder(n->type_str))
        sb_puts(&t, n->type_str);
    if (t.len) {
        sb_puts(out, ": ");
        sb_puts(out, sb_cstr(&t));
    }
    sb_free(&t);
}

static void append_value_suffix(sb_t *out, const ast_node_t *n)
{
    sb_t v;
    if (!n->a) return;
    sb_init(&v);
    if (literal_preview(&v, n->a)) {
        sb_puts(out, " = ");
        sb_puts(out, sb_cstr(&v));
    }
    sb_free(&v);
}

static void const_sig_to_sb(sb_t *out, const ast_node_t *n)
{
    sb_puts(out, "const ");
    sb_puts(out, n->name ? n->name : "");
    append_type_anno(out, n);
    append_value_suffix(out, n);
}

static void var_sig_to_sb(sb_t *out, const ast_node_t *n)
{
    sb_puts(out, n->is_mut ? "mut var " : "var ");
    sb_puts(out, n->name ? n->name : "");
    append_type_anno(out, n);
    append_value_suffix(out, n);
}

static void field_sig_to_sb(sb_t *out, const ast_node_t *n)
{
    sb_puts(out, n->name ? n->name : "");
    append_type_anno(out, n);
    append_value_suffix(out, n);
}

static void enum_member_sig_to_sb(sb_t *out, const ast_node_t *n)
{
    sb_t v;
    sb_puts(out, n->name ? n->name : "");
    sb_init(&v);
    if (literal_preview(&v, n->a)) {
        sb_puts(out, " = ");
        sb_puts(out, sb_cstr(&v));
    }
    sb_free(&v);
}

/* ==========================================================================
 * Doc-comment extraction
 *
 * Comment tokens are grouped into blocks by physical line adjacency between
 * consecutive comment tokens; any non-comment token in between is ignored,
 * since only comment-to-comment adjacency decides grouping. Each block is
 * keyed by the line it ends on, and a declaration starting on end_line + 1
 * claims it.
 * ========================================================================== */

typedef struct {
    int end_line;
    vec_t texts; /* doc_pair_t* */
} doc_block_t;

/* "// text" / "/// text" -> "text" */
static const char *strip_line_comment(arena_t *a, const char *raw)
{
    size_t n = strlen(raw), i = 0;
    const char *t;
    size_t tn;
    while (i < n && raw[i] == '/')
        i++;
    trim_span(raw + i, n - i, &t, &tn);
    return arena_strndup(a, t, tn);
}

/* A block comment -> its inner text, with the open/close markers and any
 * doc-style run of leading stars removed, and one leading '*' (JSDoc style)
 * stripped from each interior line. */
static const char *strip_block_comment(arena_t *a, const char *raw)
{
    size_t n = strlen(raw);
    size_t start = 0, stop = n;
    line_iter_t it;
    const char *line, *t;
    size_t llen, tn;
    sb_t b;
    bool first = true;
    const char *res;

    if (n >= 2 && raw[0] == '/' && raw[1] == '*') start = 2;
    while (start < n && raw[start] == '*')
        start++;
    if (stop >= 2 && raw[stop - 1] == '/' && raw[stop - 2] == '*') stop -= 2;
    /* An empty block comment leaves the opening and closing markers
     * overlapping (the star-skipping loop above can run past the closing
     * marker); clamp rather than underflow the length. */
    if (stop < start) stop = start;

    sb_init(&b);
    li_init(&it, raw + start, stop - start);
    while (li_next(&it, &line, &llen)) {
        trim_span(line, llen, &t, &tn);
        if (tn > 0 && t[0] == '*') trim_span(t + 1, tn - 1, &t, &tn);
        if (!first) sb_putc(&b, '\n');
        sb_put_span(&b, t, tn);
        first = false;
    }
    trim_span(sb_cstr(&b), b.len, &t, &tn);
    res = arena_strndup(a, t, tn);
    sb_free(&b);
    return res;
}

/*
 * Splits a block's joined text on "@lang" tag lines into [lang, text] pairs.
 * Text with no tag at all becomes a single pair with lang "" - the default
 * bucket the viewer falls back to.
 */
static void split_lang_sections(arena_t *a, const char *text, size_t len, vec_t *out)
{
    line_iter_t it;
    const char *line;
    size_t llen;
    const char *cur_lang = "";
    sb_t cur;
    bool has_any = false;
    const char *t;
    size_t tn;

    sb_init(&cur);
    li_init(&it, text, len);
    while (li_next(&it, &line, &llen)) {
        const char *tag = NULL;
        const char *rest = NULL;
        size_t restlen = 0;
        bool is_tag = false;
        const char *tl;
        size_t tlen;

        trim_span(line, llen, &tl, &tlen);
        if (tlen > 1 && tl[0] == '@') {
            size_t j = 1;
            while (j < tlen && doc_is_alpha(tl[j]))
                j++;
            if (j > 1) {
                tag = arena_strndup(a, tl + 1, j - 1);
                trim_span(tl + j, tlen - j, &rest, &restlen);
                is_tag = true;
            }
        }

        if (is_tag) {
            if (has_any) {
                trim_span(sb_cstr(&cur), cur.len, &t, &tn);
                add_pair_n(a, out, cur_lang, t, tn);
            }
            cur_lang = tag;
            sb_free(&cur);
            sb_init(&cur);
            sb_put_span(&cur, rest, restlen);
            has_any = true;
        } else if (has_any || llen > 0) {
            /* The untrimmed line, so indentation inside a doc block survives. */
            if (cur.len > 0) sb_putc(&cur, '\n');
            sb_put_span(&cur, line, llen);
            has_any = true;
        }
    }
    if (has_any) {
        trim_span(sb_cstr(&cur), cur.len, &t, &tn);
        add_pair_n(a, out, cur_lang, t, tn);
    }
    sb_free(&cur);
}

static void finalize_block(arena_t *a, vec_t *blocks, const vec_t *raw, int end_line)
{
    sb_t joined;
    doc_block_t *cb;
    size_t i;

    if (raw->len == 0) return;

    sb_init(&joined);
    for (i = 0; i < raw->len; i++) {
        if (i) sb_putc(&joined, '\n');
        sb_puts(&joined, (const char *)raw->data[i]);
    }
    cb = (doc_block_t *)arena_alloc(a, sizeof *cb);
    cb->end_line = end_line;
    vec_init(&cb->texts);
    split_lang_sections(a, sb_cstr(&joined), joined.len, &cb->texts);
    vec_push(a, blocks, cb);
    sb_free(&joined);
}

static void collect_comment_blocks(arena_t *a, const token_stream_t *ts, vec_t *blocks)
{
    size_t n = token_stream_count(ts), i;
    vec_t raw;
    /* Far enough below any real line number that the first line comment can
     * never look adjacent to a preceding block. */
    int last_end = -1000;

    vec_init(&raw);
    for (i = 0; i < n; i++) {
        const token_t *t = token_stream_at(ts, i);
        if (t->kind == TK_COMMENT_BLOCK) {
            vec_t single;
            finalize_block(a, blocks, &raw, last_end);
            raw.len = 0;
            vec_init(&single);
            vec_push(a, &single, CONST_CAST(strip_block_comment(a, t->lexeme)));
            finalize_block(a, blocks, &single, (int)t->span.end.line);
            last_end = -1000;
        } else if (t->kind == TK_COMMENT_LINE) {
            int this_begin = (int)t->span.begin.line;
            if (raw.len > 0 && this_begin != last_end + 1) {
                finalize_block(a, blocks, &raw, last_end);
                raw.len = 0;
            }
            vec_push(a, &raw, CONST_CAST(strip_line_comment(a, t->lexeme)));
            last_end = (int)t->span.end.line;
        }
    }
    finalize_block(a, blocks, &raw, last_end);
}

static void attach_doc(doc_db_t *db, const vec_t *blocks, doc_decl_t *d, int begin_line)
{
    size_t i, j;
    for (i = 0; i < blocks->len; i++) {
        const doc_block_t *cb = (const doc_block_t *)blocks->data[i];
        if (cb->end_line != begin_line - 1) continue;
        for (j = 0; j < cb->texts.len; j++) {
            const doc_pair_t *p = (const doc_pair_t *)cb->texts.data[j];
            add_pair(db->arena, &d->doc, p->lang, p->text);
        }
        return;
    }
}

/* ==========================================================================
 * AST walk
 * ========================================================================== */

static void walk_member(doc_db_t *db, const ast_node_t *m, int file_idx,
                        doc_decl_t *parent, const vec_t *blocks)
{
    doc_kind_t kind;
    sb_t sig;
    doc_decl_t *d;
    int line;

    if (is_placeholder(m->name)) return;

    if (m->kind == AST_FIELD)
        kind = DOC_FIELD;
    else if (m->kind == AST_FUNC_DEF)
        kind = DOC_FUNC;
    else
        return;

    sb_init(&sig);
    if (kind == DOC_FIELD)
        field_sig_to_sb(&sig, m);
    else
        func_sig_to_sb(&sig, m);

    line = (int)m->span.begin.line;
    d = db_add_decl(db, kind, m->name, sb_cstr(&sig), m->is_pub, m->is_deprecated,
                    file_idx, line, parent);
    sb_free(&sig);
    attach_aliases(db, m, d);
    attach_doc(db, blocks, d, line);
}

static void walk_top(doc_db_t *db, const ast_node_t *n, int file_idx, const vec_t *blocks)
{
    doc_kind_t kind;
    sb_t sig;
    doc_decl_t *d;
    int line;
    size_t i;

    /* Parse wreckage rather than API: p_name() substitutes "<error>" for a name
     * it could not read, and whatever the parser then recovered around it -
     * parameters, types, even which declaration this is - is guesswork. Drop
     * the row entirely; its members are no more trustworthy than its name. The
     * file still counts as a warning, so the run reports that something failed
     * to parse instead of silently publishing nonsense. */
    if (is_placeholder(n->name)) return;

    sb_init(&sig);
    switch (n->kind) {
    case AST_CONST_DECL:
        kind = DOC_CONST;
        const_sig_to_sb(&sig, n);
        break;
    case AST_VAR_DECL:
        kind = DOC_VAR;
        var_sig_to_sb(&sig, n);
        break;
    case AST_TYPE_ALIAS:
        kind = DOC_TYPE;
        sb_puts(&sig, "type ");
        sb_puts(&sig, n->name);
        if (n->type) {
            sb_t t;
            sb_init(&t);
            if (render_type(&t, n->type)) {
                sb_puts(&sig, " = ");
                sb_puts(&sig, sb_cstr(&t));
            }
            sb_free(&t);
        }
        break;
    case AST_FUNC_DEF:
        kind = DOC_FUNC;
        func_sig_to_sb(&sig, n);
        break;
    case AST_STRUCT_DEF:
        kind = DOC_STRUCT;
        sb_puts(&sig, "struct ");
        sb_puts(&sig, n->name ? n->name : "");
        typarams_to_sb(&sig, n);
        break;
    case AST_INTERFACE_DEF:
        kind = DOC_INTERFACE;
        sb_puts(&sig, "interface ");
        sb_puts(&sig, n->name ? n->name : "");
        typarams_to_sb(&sig, n);
        break;
    case AST_ENUM_DEF:
        kind = DOC_ENUM;
        sb_puts(&sig, "enum ");
        sb_puts(&sig, n->name ? n->name : "");
        break;
    case AST_IMPL_DEF:
        kind = DOC_IMPL;
        sb_puts(&sig, "impl ");
        sb_puts(&sig, n->name);
        if (n->type) {
            sb_t t;
            sb_init(&t);
            if (render_type(&t, n->type)) {
                sb_puts(&sig, " on ");
                sb_puts(&sig, sb_cstr(&t));
            }
            sb_free(&t);
        }
        break;
    default:
        sb_free(&sig);
        return;
    }

    line = (int)n->span.begin.line;
    d = db_add_decl(db, kind, n->name, sb_cstr(&sig), n->is_pub, n->is_deprecated,
                    file_idx, line, NULL);
    sb_free(&sig);
    attach_aliases(db, n, d);
    attach_doc(db, blocks, d, line);

    if (n->kind == AST_STRUCT_DEF || n->kind == AST_INTERFACE_DEF ||
        n->kind == AST_IMPL_DEF) {
        for (i = 0; i < n->list.len; i++)
            walk_member(db, (const ast_node_t *)n->list.data[i], file_idx, d, blocks);
    } else if (n->kind == AST_ENUM_DEF) {
        for (i = 0; i < n->list.len; i++) {
            const ast_node_t *m = (const ast_node_t *)n->list.data[i];
            sb_t msig;
            doc_decl_t *md;
            int mline = (int)m->span.begin.line;
            sb_init(&msig);
            enum_member_sig_to_sb(&msig, m);
            /* Enum members have no visibility of their own: they are as public
             * as the enum that contains them. */
            md = db_add_decl(db, DOC_ENUM_MEMBER, m->name, sb_cstr(&msig), true, false,
                             file_idx, mline, d);
            sb_free(&msig);
            attach_doc(db, blocks, md, mline);
        }
    }
}

/* ==========================================================================
 * HTML rendering
 * ========================================================================== */

static const char *doc_kind_name(doc_kind_t k)
{
    switch (k) {
    case DOC_CONST:
        return "const";
    case DOC_VAR:
        return "var";
    case DOC_TYPE:
        return "type";
    case DOC_STRUCT:
        return "struct";
    case DOC_INTERFACE:
        return "interface";
    case DOC_ENUM:
        return "enum";
    case DOC_IMPL:
        return "impl";
    case DOC_FUNC:
        return "func";
    case DOC_FIELD:
        return "field";
    default:
        return "member";
    }
}

/*
 * Escapes a string into a double-quoted JS/JSON literal. '/' is escaped too,
 * so no doc text can ever spell "</script>" and break out of the surrounding
 * <script> block.
 */
static void emit_js_str(sb_t *out, const char *s)
{
    const unsigned char *p = (const unsigned char *)(s ? s : "");
    sb_putc(out, '"');
    for (; *p; p++) {
        switch (*p) {
        case '\n':
            sb_puts(out, "\\n");
            break;
        case '\r':
            sb_puts(out, "\\r");
            break;
        case '\t':
            sb_puts(out, "\\t");
            break;
        case '"':
            sb_puts(out, "\\\"");
            break;
        case '\\':
            sb_puts(out, "\\\\");
            break;
        case '/':
            sb_puts(out, "\\/");
            break;
        default:
            if (*p < 32)
                sb_printf(out, "\\u%04x", (unsigned int)*p);
            else
                sb_putc(out, (char)*p);
            break;
        }
    }
    sb_putc(out, '"');
}

static void emit_pairs(sb_t *out, const vec_t *pairs)
{
    size_t i;
    sb_putc(out, '[');
    for (i = 0; i < pairs->len; i++) {
        const doc_pair_t *p = (const doc_pair_t *)pairs->data[i];
        if (i) sb_putc(out, ',');
        sb_putc(out, '[');
        emit_js_str(out, p->lang);
        sb_putc(out, ',');
        emit_js_str(out, p->text);
        sb_putc(out, ']');
    }
    sb_putc(out, ']');
}

static void emit_members(sb_t *out, const vec_t *members)
{
    size_t i;
    sb_putc(out, '[');
    for (i = 0; i < members->len; i++) {
        if (i) sb_putc(out, ',');
        sb_printf(out, "%d", ((const doc_decl_t *)members->data[i])->self);
    }
    sb_putc(out, ']');
}

static void emit_decl(sb_t *out, const doc_decl_t *d)
{
    sb_puts(out, "{\"k\":");
    emit_js_str(out, doc_kind_name(d->kind));
    sb_puts(out, ",\"n\":");
    emit_js_str(out, d->name);
    sb_puts(out, ",\"s\":");
    emit_js_str(out, d->sig);
    sb_puts(out, ",\"p\":");
    sb_puts(out, d->is_pub ? "true" : "false");
    sb_puts(out, ",\"dep\":");
    sb_puts(out, d->is_deprecated ? "true" : "false");
    sb_printf(out, ",\"f\":%d", d->file_idx);
    sb_printf(out, ",\"l\":%d", d->line);
    sb_printf(out, ",\"par\":%d", d->parent);
    sb_puts(out, ",\"a\":");
    emit_pairs(out, &d->aliases);
    sb_puts(out, ",\"doc\":");
    emit_pairs(out, &d->doc);
    sb_puts(out, ",\"m\":");
    emit_members(out, &d->members);
    sb_putc(out, '}');
}

static void emit_db_json(sb_t *out, const doc_db_t *db)
{
    size_t i;
    sb_puts(out, "{\"files\":[");
    for (i = 0; i < db->files.len; i++) {
        if (i) sb_putc(out, ',');
        emit_js_str(out, (const char *)db->files.data[i]);
    }
    sb_puts(out, "],\"decls\":[");
    for (i = 0; i < db->decls.len; i++) {
        if (i) sb_putc(out, ',');
        emit_decl(out, (const doc_decl_t *)db->decls.data[i]);
    }
    sb_puts(out, "]}");
}

static void put_lines(sb_t *out, const char *const *lines)
{
    size_t i;
    for (i = 0; lines[i] != NULL; i++)
        sb_puts(out, lines[i]);
}

/* ==========================================================================
 * Public API
 * ========================================================================== */

doc_db_t *docgen_db_new(void)
{
    doc_db_t *db = (doc_db_t *)malloc(sizeof *db);
    if (!db) abort();
    db->arena = arena_new(1 << 20);
    vec_init(&db->decls);
    vec_init(&db->files);
    return db;
}

void docgen_db_free(doc_db_t *db)
{
    if (!db) return;
    arena_free(db->arena);
    free(db);
}

bool docgen_parse_file(doc_db_t *db, const langpack_t *pack, const char *path,
                       const char *rel, const char *const *defines, int ndefines)
{
    arena_t *a;
    logger_t *log;
    source_file_t *src;
    const langpack_t *modpack;
    token_stream_t *toks = NULL, *toks2 = NULL;
    ast_node_t *program = NULL;
    cc_table_t *cc;
    vec_t blocks;
    bool lok, pok;
    int file_idx;
    size_t i;

    a = arena_new(DOC_FILE_ARENA);
    /* Scoped to this arena: the semantic pass registers every package source it
     * loads with the logger, which keeps borrowed pointers into `a`. Documenting
     * is not building, so nothing is worth reporting anyway - a file that does
     * not compile still documents whatever it could parse. */
    log = logger_new(stderr, LOG_OFF, false);
    src = source_load(a, path);
    if (!src) {
        logger_free(log);
        arena_free(a);
        return false;
    }
    file_idx = db_add_file(db, rel);
    modpack = langpack_detect(a, src, pack);

    lok = lexer_run(a, log, modpack, src, &toks);
    pok = parser_run(a, log, toks, &program);

    cc = cc_table_build(a, NULL, defines, ndefines);
    if (program) {
        cc_prune_program(a, log, path, cc, program);
        /* Best effort: a file that fails type checking still documents fine,
         * and running this is what fills in type_str for ':=' declarations. */
        sema_run(a, log, program, path, langpack_code(modpack), cc);
    }

    /* Second, comment-preserving lex - never handed to the parser. */
    lexer_run_ex(a, log, modpack, src, true, &toks2);
    vec_init(&blocks);
    collect_comment_blocks(a, toks2, &blocks);

    if (program) {
        for (i = 0; i < program->list.len; i++)
            walk_top(db, (const ast_node_t *)program->list.data[i], file_idx, &blocks);
    }

    logger_free(log);
    arena_free(a);
    return lok && pok;
}

void docgen_render(const doc_db_t *db, const char *title, sb_t *out)
{
    sb_puts(out, "<!doctype html>\n<html lang=\"en\">\n<head>\n");
    sb_puts(out, "<meta charset=\"utf-8\">\n");
    sb_puts(out, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
    sb_puts(out, "<title>");
    sb_puts(out, title);
    sb_puts(out, " - API Documentation</title>\n");
    sb_puts(out, "<style>");
    put_lines(out, doc_css_lines);
    sb_puts(out, "</style>\n</head>\n<body>\n");
    put_lines(out, doc_body_lines);
    sb_puts(out, "\n<script>\nvar DOC_TITLE = ");
    emit_js_str(out, title);
    sb_puts(out, ";\nvar DOC = ");
    emit_db_json(out, db);
    sb_puts(out, ";\n");
    put_lines(out, doc_js_lines);
    sb_puts(out, "\n</script>\n</body>\n</html>\n");
}
