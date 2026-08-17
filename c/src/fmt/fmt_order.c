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
#include "core/sal_format.h"
#include "fmt/fmt_order.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "ast/ast.h"
#include "langpack/langpack.h"
#include "semantic/sema_internal.h"

typedef struct {
    ast_node_t *node;
    ast_order_phase_t phase;
    size_t ustart;
    size_t uend;
} ord_item_t;

static void sb_append_bytes(sb_t *s, const char *bytes, size_t n)
{
    size_t i = 0;
    for (; i < n; i++)
        sb_putc(s, bytes[i]);
}

/* Right-trim trailing whitespace/newlines from a raw [start,end) range. */
static size_t rtrim_end(const char *text, size_t start, size_t end)
{
    while (end > start) {
        char c = text[end - 1];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') break;
        end--;
    }
    return end;
}

static bool is_modifier_kind(token_kind_t k)
{
    return k == TK_KW_PUB || k == TK_KW_DEPRECATED || k == TK_KW_INLINE ||
           k == TK_KW_NOINLINE || k == TK_KW_PURE || k == TK_KW_NORET;
}

static bool is_entry_point(const ast_node_t *d, const char *entry)
{
    if (!d || d->kind != AST_FUNC_DEF || !d->name) return false;
    if (strcmp(d->name, "main") == 0) return true;
    return entry && strcmp(d->name, entry) == 0;
}

static bool is_movable(const ast_node_t *d, const char *entry)
{
    if (!d || d->synthetic) return false;
    if (d->is_extern)
        return false; /* member of an 'extern: end' block; no wrapper
                         span exists to safely relocate it out of */
    /* The program's entry point is conventionally placed last regardless of
     * visibility and is exempt from the pub-ordering rule (it isn't part of
     * the file's public API surface) - matches sema_check_toplevel_order's
     * check_pub_order exemption exactly, so never relocate it either. */
    if (is_entry_point(d, entry)) return false;
    return ast_toplevel_order_phase(d) != AST_ORDER_NONE;
}

static bool line_is_blank(const char *text, size_t a, size_t b)
{
    size_t i = a;
    for (; i < b; i++)
        if (text[i] != ' ' && text[i] != '\t' && text[i] != '\r') return false;
    return true;
}

/* True if the line [a,b) is a '//' comment or an '@lang "..."' annotation
 * line. Sets *maybe_block if the line looks like it could be part of a
 * block comment (either opens or closes one) - the caller should treat
 * that as "don't know, don't touch" rather than guess. */
static bool line_is_comment_or_meta(const char *text, size_t a, size_t b,
                                    bool *maybe_block)
{
    size_t i = a;
    while (i < b && (text[i] == ' ' || text[i] == '\t'))
        i++;
    if (i + 1 < b && text[i] == '/' && text[i + 1] == '/') return true;
    if (i < b && text[i] == '@') return true;
    {
        size_t j = a;
        for (; j + 1 < b; j++) {
            if ((text[j] == '/' && text[j + 1] == '*') ||
                (text[j] == '*' && text[j + 1] == '/')) {
                *maybe_block = true;
                return false;
            }
        }
    }
    return false;
}

static size_t prev_line_start(const char *text, size_t pos)
{
    if (pos < 2) return 0;
    size_t i = pos - 1;
    if (text[i] != '\n') return pos;
    if (i == 0) return 0;
    i--;
    while (i > 0 && text[i - 1] != '\n')
        i--;
    return i;
}

/* Walk backward from `begin` (must be a line start) over a blank-line-free
 * run of immediately-preceding comment/@meta lines, extending the movable
 * unit's start upward so those lines travel with the declaration when it's
 * relocated. Stops at the first blank line or non-trivia line. */
static size_t scan_leading_trivia(const char *text, size_t begin, bool *bail)
{
    size_t cur = begin;
    for (;;) {
        if (cur == 0) break;
        size_t pstart = prev_line_start(text, cur);
        size_t pend = cur - 1;
        if (line_is_blank(text, pstart, pend)) break;
        bool maybe_block = false;
        if (!line_is_comment_or_meta(text, pstart, pend, &maybe_block)) {
            if (maybe_block) *bail = true;
            break;
        }
        if (pstart == cur) break;
        cur = pstart;
    }
    return cur;
}

/* Extend forward past a trailing same-line '// comment' (if any) so it
 * doesn't get orphaned when the declaration before it moves. */
static size_t extend_trailing_comment(const char *text, size_t len, size_t end)
{
    size_t i = end;
    while (i < len && (text[i] == ' ' || text[i] == '\t'))
        i++;
    if (i + 1 < len && text[i] == '/' && text[i + 1] == '/') {
        while (i < len && text[i] != '\n')
            i++;
        return i;
    }
    return end;
}

static uint32_t adjusted_begin_index(const token_stream_t *toks, size_t tok_idx)
{
    size_t i = tok_idx;
    while (i > 0 && is_modifier_kind(token_stream_at(toks, i - 1)->kind))
        i--;
    return token_stream_at(toks, i)->span.begin.index;
}

/*
 * An AST_IMPORT node's span starts *after* its 'import' keyword, because
 * parse_imports_into consumes the keyword before building any entry - unlike
 * AST_LINK, whose node is created while the cursor still sits on 'link'. On
 * top of that, one `import "a" "b" "c"` statement yields three sibling
 * AST_IMPORT nodes sharing that single keyword.
 *
 * Both fall out of one observation: an entry begins a new import *statement*
 * exactly when the token in front of it is the keyword. That identifies the
 * statement's true start, and every following entry that is not so preceded
 * is a continuation to fold into the same movable unit.
 */
static bool import_starts_statement(const token_stream_t *toks, size_t tok_idx)
{
    return tok_idx > 0 && token_stream_at(toks, tok_idx - 1)->kind == TK_KW_IMPORT;
}

static size_t find_token_for_begin(const token_stream_t *toks, size_t *cursor,
                                   uint32_t begin_index)
{
    size_t n = token_stream_count(toks);
    size_t i = *cursor;
    for (; i < n; i++) {
        if (token_stream_at(toks, i)->span.begin.index == begin_index) {
            *cursor = i;
            return i;
        }
    }
    return n; /* not found: shouldn't happen for a real top-level node */
}

/*
 * Hoist a misplaced `package` line to the top of the file.
 *
 * This one cannot be done with the rest: a package below the first statement
 * is rejected by the *parser* ("only as the very first statement"), so there
 * is no tree left to reorder. It therefore runs on the token stream, before
 * any parse is attempted, and rewrites the text so the parse can succeed.
 *
 * Only `@meta` annotations may legitimately precede a package declaration, so
 * anything else in front of the keyword means it is misplaced. The moved unit
 * carries its own leading comments and annotations along with it.
 *
 * Returns true when `out` was rewritten; false leaves it untouched.
 */
static bool hoist_package(const char *text, size_t len, const token_stream_t *toks,
                          sb_t *out)
{
    size_t n = token_stream_count(toks);
    size_t kw = n;
    size_t i = 0;
    for (; i < n; i++) {
        token_kind_t k = token_stream_at(toks, i)->kind;
        if (k == TK_KW_PACKAGE) {
            kw = i;
            break;
        }
        if (k == TK_META || k == TK_STMT_END || k == TK_EOF) continue;
        /* A real statement came first - the package below it is misplaced. */
        for (; i < n; i++)
            if (token_stream_at(toks, i)->kind == TK_KW_PACKAGE) {
                kw = i;
                break;
            }
        break;
    }
    if (kw >= n) return false;
    /* Already in front of everything but annotations: nothing to do. */
    {
        bool only_meta = true;
        size_t j = 0;
        for (; j < kw; j++) {
            token_kind_t k = token_stream_at(toks, j)->kind;
            if (k != TK_META && k != TK_STMT_END) {
                only_meta = false;
                break;
            }
        }
        if (only_meta) return false;
    }
    /* The unit is `package <name>` plus whatever comments and annotations sit
     * directly above it. The name token follows the keyword. */
    if (kw + 1 >= n) return false;
    {
        bool bail = false;
        size_t ustart =
            scan_leading_trivia(text, token_stream_at(toks, kw)->span.begin.index, &bail);
        size_t uend = token_stream_at(toks, kw + 1)->span.end.index;
        if (bail) return false;
        uend = rtrim_end(text, ustart, uend);
        uend = extend_trailing_comment(text, len, uend);
        if (uend <= ustart) return false;
        /*
         * Insert in front of the first real declaration rather than at byte
         * 0: a file normally opens with a licence banner, and a package line
         * shoved above that reads as though it belongs to nothing. The
         * banner is a block comment, which scan_leading_trivia deliberately
         * refuses to walk into, so the first declaration's own leading `//`
         * comments are the boundary - the package lands after the banner and
         * before that declaration's comment, where a hand-written one would.
         */
        size_t ins = 0;
        {
            /* Skip a leading block-comment banner, if there is one. It cannot
             * be found by walking backward from the first declaration the way
             * everything else here does: scan_leading_trivia refuses to enter
             * a block comment on purpose, so it stops at the banner rather
             * than stepping over it. Scanning forward from the top instead is
             * unambiguous - the banner is whatever block comment the file
             * opens with. */
            size_t p = 0;
            while (p < len && (text[p] == ' ' || text[p] == '\t' || text[p] == '\r' ||
                               text[p] == '\n'))
                p++;
            if (p + 1 < len && text[p] == '/' && text[p + 1] == '*') {
                size_t q = p + 2;
                while (q + 1 < len && !(text[q] == '*' && text[q + 1] == '/'))
                    q++;
                if (q + 1 < len) {
                    q += 2;
                    while (q < len && text[q] != '\n')
                        q++;
                    if (q < len) q++;
                    while (q < len && (text[q] == '\r' || text[q] == '\n'))
                        q++;
                    if (q < ustart) ins = q;
                }
            }
        }
        sb_append_bytes(out, text, ins);
        sb_append_bytes(out, text + ustart, uend - ustart);
        sb_puts(out, "\n\n");
        sb_append_bytes(out, text + ins, ustart - ins);
        /* Skip the blank line the moved block left behind. */
        {
            size_t after = uend;
            while (after < len &&
                   (text[after] == ' ' || text[after] == '\t' || text[after] == '\r'))
                after++;
            if (after < len && text[after] == '\n') after++;
            sb_append_bytes(out, text + after, len - after);
        }
    }
    return true;
}

static int item_sort_key(const ord_item_t *it)
{
    int secondary = (it->phase == AST_ORDER_FUNC && it->node->is_pub) ? 1 : 0;
    return (int)it->phase * 2 + secondary;
}

/* Stable sort (small n; insertion sort is simplest and is stable). */
static void stable_sort_items(ord_item_t *items, size_t n)
{
    size_t i = 1;
    for (; i < n; i++) {
        ord_item_t key = items[i];
        int kkey = item_sort_key(&key);
        size_t j = i;
        while (j > 0 && item_sort_key(&items[j - 1]) > kkey) {
            items[j] = items[j - 1];
            j--;
        }
        items[j] = key;
    }
}

static void append_notes_from_diag(arena_t *a, diag_engine_t *diag, vec_t *notes)
{
    if (!notes) return;
    size_t i = 0;
    for (; i < diag->items.len; i++) {
        diag_t *d = (diag_t *)diag->items.data[i];
        if (!d) continue;
        char buf[768];
        sal_snprintf(buf, sizeof buf, "%s:%u: %s", d->file ? d->file : "?",
                     d->span.begin.line, d->message);
        vec_push(a, notes, (void *)arena_strdup(a, buf));
    }
}

bool fmt_reorder_toplevel(arena_t *a, logger_t *log, const langpack_t *pack,
                          const source_file_t *src, const char *file, sb_t *out,
                          vec_t *notes)
{
    const char *text = src->text;
    size_t len = src->len;

    token_stream_t *toks = NULL;
    if (!lexer_run(a, log, pack, src, &toks)) {
        sb_puts(out, text);
        return false;
    }
    /* Runs before the parse, not after: a misplaced package is a *parse*
     * error, so leaving it in place would mean there is no tree to reorder
     * and every other fix in this file would be skipped too. */
    {
        sb_t hoisted;
        sb_init(&hoisted);
        if (hoist_package(text, len, toks, &hoisted)) {
            source_file_t *hsrc = (source_file_t *)arena_alloc(a, sizeof(source_file_t));
            token_stream_t *htoks = NULL;
            hsrc->path = src->path;
            hsrc->text = arena_strdup(a, sb_cstr(&hoisted));
            hsrc->len = hoisted.len;
            if (lexer_run(a, log, pack, hsrc, &htoks)) {
                src = hsrc;
                text = hsrc->text;
                len = hsrc->len;
                toks = htoks;
            }
        }
        sb_free(&hoisted);
    }
    ast_node_t *program = NULL;
    if (!parser_run(a, log, toks, &program) || !program) {
        sb_puts(out, text);
        return false;
    }

    const char *entry = langpack_entry_for(langpack_code(pack));

    size_t n_list = program->list.len;
    ord_item_t *items = (ord_item_t *)arena_alloc(a, sizeof(ord_item_t) * (n_list + 1));
    size_t n_items = 0;
    size_t tok_cursor = 0;

    /* Build the flat sequence of top-level "slots": either a movable
     * declaration (with its true, trivia-inclusive text span) or a barrier
     * (is_extern member, top-level 'if', the program's entry point, or
     * anything this scan can't confidently classify) that always stays
     * exactly where it is. */
    size_t i = 0;
    for (; i < n_list; i++) {
        ast_node_t *d = (ast_node_t *)program->list.data[i];
        if (!d || d->synthetic) continue;
        if (!is_movable(d, entry)) {
            items[n_items].node = d;
            items[n_items].phase = AST_ORDER_NONE;
            items[n_items].ustart = items[n_items].uend = 0;
            n_items++;
            continue;
        }
        size_t tok_idx = find_token_for_begin(toks, &tok_cursor, d->span.begin.index);
        if (tok_idx >= token_stream_count(toks)) {
            items[n_items].node = d;
            items[n_items].phase = AST_ORDER_NONE;
            items[n_items].ustart = items[n_items].uend = 0;
            n_items++;
            continue;
        }
        if (d->kind == AST_IMPORT && !import_starts_statement(toks, tok_idx)) {
            /* A continuation entry of `import "a" "b"`: it has no keyword of
             * its own, so it cannot be a unit. Stretch the statement's unit
             * over it instead, and let it travel as one piece. */
            if (n_items > 0 && items[n_items - 1].phase == AST_ORDER_IMPORT) {
                size_t e = rtrim_end(text, items[n_items - 1].ustart, d->span.end.index);
                e = extend_trailing_comment(text, len, e);
                if (e > items[n_items - 1].uend) items[n_items - 1].uend = e;
                continue;
            }
            items[n_items].node = d;
            items[n_items].phase = AST_ORDER_NONE;
            items[n_items].ustart = items[n_items].uend = 0;
            n_items++;
            continue;
        }
        uint32_t adj_begin = adjusted_begin_index(toks, tok_idx);
        if (d->kind == AST_IMPORT)
            adj_begin = token_stream_at(toks, tok_idx - 1)->span.begin.index;
        bool bail = false;
        size_t ustart = scan_leading_trivia(text, adj_begin, &bail);
        if (bail) {
            items[n_items].node = d;
            items[n_items].phase = AST_ORDER_NONE;
            items[n_items].ustart = items[n_items].uend = 0;
            n_items++;
            continue;
        }
        /* d->span.end can land at column 1 of the FOLLOWING line (a
         * statement-terminator artifact), which would otherwise make this
         * item's extracted text quietly swallow the blank line that
         * separates it from whatever comes next. Normalize back to the
         * true end of this item's own content first. */
        size_t uend = rtrim_end(text, ustart, d->span.end.index);
        uend = extend_trailing_comment(text, len, uend);
        items[n_items].node = d;
        items[n_items].phase = ast_toplevel_order_phase(d);
        items[n_items].ustart = ustart;
        items[n_items].uend = uend;
        n_items++;
    }

    /* Walk maximal runs of movable items, reordering each run in place
     * (verbatim text, never touching anything outside a run that actually
     * needs to change). */
    sb_t result;
    sb_init(&result);
    size_t cursor = 0;
    size_t r = 0;
    while (r < n_items) {
        if (items[r].phase == AST_ORDER_NONE) {
            r++;
            continue;
        }
        size_t run_start = r;
        size_t run_end = r;
        while (run_end + 1 < n_items && items[run_end + 1].phase != AST_ORDER_NONE)
            run_end++;
        size_t run_len = run_end - run_start + 1;

        ord_item_t *sorted = (ord_item_t *)arena_alloc(a, sizeof(ord_item_t) * run_len);
        memcpy(sorted, &items[run_start], sizeof(ord_item_t) * run_len);
        stable_sort_items(sorted, run_len);

        bool changed = false;
        {
            size_t k = 0;
            for (; k < run_len; k++)
                if (sorted[k].node != items[run_start + k].node) {
                    changed = true;
                    break;
                }
        }

        if (changed) {
            size_t region_start = items[run_start].ustart;
            size_t region_end = items[run_end].uend;
            sb_append_bytes(&result, text + cursor, region_start - cursor);
            {
                size_t k = 0;
                for (; k < run_len; k++) {
                    size_t pstart = sorted[k].ustart;
                    size_t pend = rtrim_end(text, pstart, sorted[k].uend);
                    sb_append_bytes(&result, text + pstart, pend - pstart);
                    if (k + 1 < run_len) sb_puts(&result, "\n\n");
                }
            }
            cursor = region_end;
        }
        r = run_end + 1;
    }
    sb_append_bytes(&result, text + cursor, len - cursor);

    sb_puts(out, sb_cstr(&result));
    sb_free(&result);

    /* Verify: re-parse the fixed text and run the real ordering checker
     * against it, so any leftover problem this pass couldn't (or
     * shouldn't) fix on its own is reported precisely, using the same
     * diagnostics a normal build would show. */
    if (notes) {
        source_file_t fixed_src = *src;
        fixed_src.text = out->data;
        fixed_src.len = out->len;
        token_stream_t *ftoks = NULL;
        ast_node_t *fprogram = NULL;
        if (lexer_run(a, log, pack, &fixed_src, &ftoks) &&
            parser_run(a, log, ftoks, &fprogram) && fprogram) {
            sema_t s;
            memset(&s, 0, sizeof(s));
            s.a = a;
            s.log = log;
            s.diag = diag_new(a, log, PH_SEMANTIC);
            s.file = file;
            s.lang = langpack_code(pack);
            sema_check_toplevel_order(&s, fprogram);
            append_notes_from_diag(a, s.diag, notes);
        }
    }

    return true;
}
