
#include "llvm/codegen_llvm_internal.h"

const char *ll_meta_add(ll_t *ll, const char *text)
{
    const char *id = ll_fmt(ll, "!%d", ll->meta_n++);
    sb_puts(ll->meta, ll_fmt(ll, "%s = %s\n", id, text));
    return id;
}

static const char *di_escape(ll_t *ll, const char *s)
{
    if (!s) return "";
    sb_t b; sb_init(&b);
    for (const char *p = s; *p; p++) {
        if (*p == '\\' || *p == '"') sb_putc(&b, '\\');
        sb_putc(&b, *p);
    }
    const char *r = arena_strdup(ll->a, sb_cstr(&b)); sb_free(&b); return r;
}

static void di_split_path(ll_t *ll, const char *path)
{
    if (!path || !path[0]) {
        ll->src_file = ll_fmt(ll, "%s.salam", ll->module);
        ll->src_dir  = ".";
        return;
    }
    const char *slash = strrchr(path, '/');
    const char *bs    = strrchr(path, '\\');
    const char *cut = slash;
    if (bs && (!slash || bs > slash)) cut = bs;
    if (cut) {
        ll->src_file = arena_strdup(ll->a, cut + 1);
        ll->src_dir  = arena_strndup(ll->a, path, (size_t)(cut - path));
    } else {
        ll->src_file = arena_strdup(ll->a, path);
        ll->src_dir  = ".";
    }
}

void ll_debug_init(ll_t *ll, const char *src_path)
{
    di_split_path(ll, src_path);
    ll->di_file = ll_meta_add(ll, ll_fmt(ll,
        "!DIFile(filename: \"%s\", directory: \"%s\")",
        di_escape(ll, ll->src_file), di_escape(ll, ll->src_dir)));
    ll->di_cu = ll_meta_add(ll, ll_fmt(ll,
        "distinct !DICompileUnit(language: DW_LANG_C99, file: %s, "
        "producer: \"salamc\", isOptimized: false, runtimeVersion: 0, "
        "emissionKind: FullDebug)", ll->di_file));
    const char *types = ll_meta_add(ll, "!{null}");
    ll->di_subty = ll_meta_add(ll, ll_fmt(ll, "!DISubroutineType(types: %s)", types));
    ll->di_flag_dwarf = ll_meta_add(ll, "!{i32 7, !\"Dwarf Version\", i32 5}");
    ll->di_flag_debug = ll_meta_add(ll, "!{i32 2, !\"Debug Info Version\", i32 3}");
}

const char *ll_debug_subprogram(ll_t *ll, const char *name, unsigned line)
{
    const char *sp = ll_meta_add(ll, ll_fmt(ll,
        "distinct !DISubprogram(name: \"%s\", scope: %s, file: %s, line: %u, "
        "type: %s, scopeLine: %u, spFlags: DISPFlagDefinition, unit: %s)",
        name, ll->di_file, ll->di_file, line, ll->di_subty, line, ll->di_cu));
    ll->cur_sp = sp;
    return sp;
}

const char *ll_debug_location(ll_t *ll, unsigned line, unsigned col)
{
    if (!ll->cur_sp) return NULL;
    return ll_meta_add(ll, ll_fmt(ll,
        "!DILocation(line: %u, column: %u, scope: %s)", line, col, ll->cur_sp));
}

void ll_debug_finalize(ll_t *ll)
{
    if (!ll->debug) return;
    sb_puts(ll->g, "\n");
    sb_puts(ll->g, ll_fmt(ll, "!llvm.module.flags = !{%s, %s}\n",
                          ll->di_flag_dwarf, ll->di_flag_debug));
    sb_puts(ll->g, ll_fmt(ll, "!llvm.dbg.cu = !{%s}\n", ll->di_cu));
    sb_puts(ll->g, sb_cstr(ll->meta));
}
