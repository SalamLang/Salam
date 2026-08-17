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

#include "llvm/codegen_llvm_internal.h"
#include "core/sal_path.h"

const char *ll_meta_add(ll_t *ll, const char *text)
{
    const char *id = ll_fmt(ll, "!%d", ll->meta_n++);
    sb_puts(ll->meta, ll_fmt(ll, "%s = %s\n", id, text));
    return id;
}

/*
 * Two-family TBAA: "any pointer" for pointer-typed struct fields, "salam data"
 * for every 16-bit-or-wider integer and both floats, nothing else tagged.
 * The families are this coarse on purpose. Salam code may reinterpret one
 * data buffer through several non-pointer views (std/encoding/binary reads a
 * stored f64 back as i64; the interpreter's value payload is written through
 * every width), so all of those must stay in ONE family. Pointer accesses are
 * tagged only when they are struct-field accesses because raw T*-view element
 * reads of pointer-sized cells are how std/core republishes argv i64s as strs;
 * an untagged access aliases everything, which keeps that legal. i8/char/bool
 * stay untagged so byte-level walks of any memory remain universal aliases.
 * What the split buys: an element store (data) can no longer clobber a
 * container's data-pointer field (any pointer), so LICM keeps `v.data` in a
 * register across loops that write elements through a &: parameter.
 */
static void ll_tbaa_ensure_common(ll_t *ll)
{
    const char *root;
    if (ll->tbaa_char) return;
    root = ll_meta_add(ll, "!{!\"salam-tbaa\"}");
    ll->tbaa_char = ll_meta_add(ll, ll_fmt(ll, "!{!\"salam bytes\", %s, i64 0}", root));
}

const char *ll_tbaa_suffix(ll_t *ll, const char *ts, bool is_field)
{
    if (!ts) return "";
    if (ll_is_ptr_ts(ts) || ll_is_str(ll, ts) || ll_is_any_fn_ts(ts)) {
        if (!is_field) return "";
        if (!ll->tbaa_ptr_sfx) {
            const char *node, *tag;
            ll_tbaa_ensure_common(ll);
            node = ll_meta_add(
                ll, ll_fmt(ll, "!{!\"any pointer\", %s, i64 0}", ll->tbaa_char));
            tag = ll_meta_add(ll, ll_fmt(ll, "!{%s, %s, i64 0}", node, node));
            ll->tbaa_ptr_sfx = ll_fmt(ll, ", !tbaa %s", tag);
        }
        return ll->tbaa_ptr_sfx;
    }
    if (!(ll_is_float(ts) || (ll_is_int(ts) && ll_int_bits(ll, ts) >= 16))) return "";
    if (!ll->tbaa_data_sfx) {
        const char *node, *tag;
        ll_tbaa_ensure_common(ll);
        node =
            ll_meta_add(ll, ll_fmt(ll, "!{!\"salam data\", %s, i64 0}", ll->tbaa_char));
        tag = ll_meta_add(ll, ll_fmt(ll, "!{%s, %s, i64 0}", node, node));
        ll->tbaa_data_sfx = ll_fmt(ll, ", !tbaa %s", tag);
    }
    return ll->tbaa_data_sfx;
}

static const char *di_escape(ll_t *ll, const char *s)
{
    if (!s) return "";
    sb_t b;
    sb_init(&b);
    {
        const char *p = s;
        for (; *p; p++) {
            if (*p == '\\' || *p == '"') sb_putc(&b, '\\');
            sb_putc(&b, *p);
        }
    }
    const char *r = arena_strdup(ll->a, sb_cstr(&b));
    sb_free(&b);
    return r;
}

static void di_split_path(ll_t *ll, const char *path)
{
    if (!path || !path[0]) {
        ll->src_file = ll_fmt(ll, "%s.salam", ll->module);
        ll->src_dir = ".";
        return;
    }
    ll->src_file = arena_strdup(ll->a, sal_path_base(path));
    {
        const char *dir = sal_path_dir(ll->a, path);
        ll->src_dir = dir[0] ? dir : ".";
    }
}

void ll_debug_init(ll_t *ll, const char *src_path)
{
    di_split_path(ll, src_path);
    ll->di_file =
        ll_meta_add(ll, ll_fmt(ll, "!DIFile(filename: \"%s\", directory: \"%s\")",
                               di_escape(ll, ll->src_file), di_escape(ll, ll->src_dir)));
    ll->di_cu = ll_meta_add(
        ll, ll_fmt(ll,
                   "distinct !DICompileUnit(language: DW_LANG_C99, file: %s, "
                   "producer: \"salam\", isOptimized: false, runtimeVersion: 0, "
                   "emissionKind: FullDebug)",
                   ll->di_file));
    const char *types = ll_meta_add(ll, "!{null}");
    ll->di_subty = ll_meta_add(ll, ll_fmt(ll, "!DISubroutineType(types: %s)", types));
    ll->di_flag_dwarf = ll_meta_add(ll, "!{i32 7, !\"Dwarf Version\", i32 5}");
    ll->di_flag_debug = ll_meta_add(ll, "!{i32 2, !\"Debug Info Version\", i32 3}");
}

const char *ll_debug_subprogram(ll_t *ll, const char *name, unsigned line)
{
    const char *sp = ll_meta_add(
        ll, ll_fmt(ll,
                   "distinct !DISubprogram(name: \"%s\", scope: %s, file: %s, line: %u, "
                   "type: %s, scopeLine: %u, spFlags: DISPFlagDefinition, unit: %s)",
                   name, ll->di_file, ll->di_file, line, ll->di_subty, line, ll->di_cu));
    ll->cur_sp = sp;
    return sp;
}

const char *ll_debug_location(ll_t *ll, unsigned line, unsigned col)
{
    if (!ll->cur_sp) return NULL;
    return ll_meta_add(ll, ll_fmt(ll, "!DILocation(line: %u, column: %u, scope: %s)",
                                  line, col, ll->cur_sp));
}

void ll_debug_finalize(ll_t *ll)
{
    /* Also the flush point for TBAA nodes, which exist without -g. */
    if (!ll->debug && ll->meta_n == 0) return;
    sb_puts(ll->g, "\n");
    if (ll->debug) {
        sb_puts(ll->g, ll_fmt(ll, "!llvm.module.flags = !{%s, %s}\n", ll->di_flag_dwarf,
                              ll->di_flag_debug));
        sb_puts(ll->g, ll_fmt(ll, "!llvm.dbg.cu = !{%s}\n", ll->di_cu));
    }
    sb_puts(ll->g, sb_cstr(ll->meta));
}
