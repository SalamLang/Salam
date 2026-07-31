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

#include "llvm/codegen_llvm_internal.h"
#include "codegen/codegen.h"
#include "i18n/i18n.h"

static const char *ll_datalayout(const char *triple)
{
    static const char *dl_x64_elf = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-"
                                    "i128:128-f80:128-n8:16:32:64-S128";
    static const char *dl_x64_win = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-"
                                    "i128:128-f80:128-n8:16:32:64-S128";
    static const char *dl_x64_mac = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-"
                                    "i128:128-f80:128-n8:16:32:64-S128";
    static const char *dl_x86_elf = "e-m:e-p:32:32-p270:32:32-p271:32:32-p272:64:64-"
                                    "i128:128-f64:32:64-f80:32-n8:16:32-S128";
    static const char *dl_a64_elf = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128";
    static const char *dl_a64_mac = "e-m:o-i64:64-i128:128-n32:64-S128";
    static const char *dl_rv64 = "e-m:e-p:64:64-i64:64-i128:128-n32:64-S128";
    if (!triple || !triple[0]) {
#if defined(__x86_64__) || defined(_M_X64)
#  if defined(_WIN32)
        return dl_x64_win;
#  elif defined(__APPLE__)
        return dl_x64_mac;
#  else
        return dl_x64_elf;
#  endif
#elif defined(__aarch64__) || defined(_M_ARM64)
#  if defined(__APPLE__)
        return dl_a64_mac;
#  else
        return dl_a64_elf;
#  endif
#elif defined(__i386__) || defined(_M_IX86)
        return dl_x86_elf;
#else
        return NULL;
#endif
    }
    {
        bool win = strstr(triple, "windows") != NULL;
        bool mac = strstr(triple, "apple") || strstr(triple, "darwin") ||
                   strstr(triple, "macos");
        if (strstr(triple, "x86_64") || strstr(triple, "amd64"))
            return win ? dl_x64_win : (mac ? dl_x64_mac : dl_x64_elf);
        if (strstr(triple, "aarch64") || strstr(triple, "arm64"))
            return mac ? dl_a64_mac : dl_a64_elf;
        if (!win && !mac &&
            (strstr(triple, "i686") || strstr(triple, "i586") || strstr(triple, "i486") ||
             strstr(triple, "i386")))
            return dl_x86_elf;
        if (strstr(triple, "riscv64")) return dl_rv64;
    }
    return NULL;
}

bool ll_target_is_windows(const char *triple)
{
    if (triple && triple[0])
        return strstr(triple, "windows") != NULL || strstr(triple, "mingw") != NULL ||
               strstr(triple, "win32") != NULL;
#if defined(_WIN32)
    return true;
#else
    return false;
#endif
}

static const char *pl_widen(ll_t *ll, sb_t *g, const char *src, const char *dst)
{
    if (ll->ptr_bits <= 32) return src;
    sb_puts(g, "  ");
    sb_puts(g, dst);
    sb_puts(g, " = sext i32 ");
    sb_puts(g, src);
    sb_puts(g, " to ");
    sb_puts(g, ll->usize);
    sb_puts(g, "\n");
    return dst;
}

static void ll_emit_prologue(ll_t *ll)
{
    sb_t *g = ll->g;
    const char *U = ll->usize;

    sb_puts(g, "; Salam -> LLVM IR (textual). usize = ");
    sb_puts(g, U);
    sb_puts(g, " (");
    sb_puts(g, ll->ptr_bits == 32 ? "32" : "64");
    sb_puts(g, "-bit target). Compile with a matching toolchain:\n");
    sb_puts(g, ";   clang prog.ll -o prog            (native host)\n");
    sb_puts(g, ";   clang -m32 / --target=i686-... prog.ll   (32-bit)\n");

    sb_puts(g, "declare i32 @printf(ptr, ...) nounwind\n");
    sb_puts(g, "declare i32 @dprintf(i32, ptr, ...) nounwind\n");
    sb_puts(g, "declare i64 @write(i32, ptr, i64) nounwind\n");
    sb_puts(g,
            ll_fmt(ll,
                   "declare %s @strlen(ptr) nounwind willreturn memory(argmem: read)\n",
                   U));
    sb_puts(g,
            "declare i32 @strcmp(ptr, ptr) nounwind willreturn memory(argmem: read)\n");
    sb_puts(g, ll_fmt(ll, "declare noalias ptr @malloc(%s) nounwind allocsize(0)\n", U));
    sb_puts(g, ll_fmt(ll, "declare noalias ptr @realloc(ptr, %s) nounwind allocsize(1)\n",
                      U));
    sb_puts(g, "declare void @free(ptr) nounwind\n");
    sb_puts(g, ll_fmt(ll, "declare ptr @memcpy(ptr, ptr, %s) nounwind\n", U));
    sb_puts(g, ll_fmt(ll, "declare ptr @memmove(ptr, ptr, %s) nounwind\n", U));
    sb_puts(g, "declare void @abort() noreturn nounwind cold\n");
    sb_puts(g, "declare void @exit(i32) noreturn nounwind\n");
    sb_puts(g, ll_fmt(ll, "declare i32 @snprintf(ptr, %s, ptr, ...) nounwind\n", U));
    sb_puts(g, "declare i64 @strtol(ptr, ptr, i32) nounwind\n");
    sb_puts(g, "declare double @strtod(ptr, ptr) nounwind\n");
    sb_puts(g,
            "declare ptr @strstr(ptr, ptr) nounwind willreturn memory(argmem: read)\n");
    sb_puts(g, "declare double @llvm.pow.f64(double, double)\n\n");

    sb_puts(
        g,
        "; `dyn Iface` fat pointer: { data, vtable } (vtable = [N x ptr] of methods)\n");
    sb_puts(g, "%dyn = type { ptr, ptr }\n\n");
}

static void ll_emit_substr(ll_t *ll)
{
    sb_t *g = ll->hg;
    const char *U = ll->usize;
    sb_puts(g, "define internal noalias ptr @salam_ll_substr(ptr %s, i32 %start, i32 "
               "%len) nounwind willreturn nofree {\nentry:\n");
    const char *L = pl_widen(ll, g, "%len", "%l");
    const char *SO = pl_widen(ll, g, "%start", "%so");
    sb_puts(g, ll_fmt(ll, "  %%t = add %s %s, 1\n", U, L));
    sb_puts(g, ll_fmt(ll, "  %%buf = call ptr @malloc(%s %%t)\n", U));
    sb_puts(g, ll_fmt(ll, "  %%src = getelementptr i8, ptr %%s, %s %s\n", U, SO));
    sb_puts(g,
            ll_fmt(ll, "  %%c = call ptr @memcpy(ptr %%buf, ptr %%src, %s %s)\n", U, L));
    sb_puts(g, ll_fmt(ll, "  %%e = getelementptr i8, ptr %%buf, %s %s\n", U, L));
    sb_puts(g, "  store i8 0, ptr %e\n  ret ptr %buf\n}\n\n");
}

static void ll_emit_strcat(ll_t *ll)
{
    const char *U = ll->usize;
    sb_puts(ll->hg,
            ll_fmt(ll,
                   "define internal noalias ptr @salam_ll_strcat(ptr %%a, ptr %%b) "
                   "nounwind willreturn nofree {\n"
                   "entry:\n"
                   "  %%la = call %s @strlen(ptr %%a)\n"
                   "  %%lb = call %s @strlen(ptr %%b)\n"
                   "  %%sum = add %s %%la, %%lb\n"
                   "  %%tot = add %s %%sum, 1\n"
                   "  %%buf = call ptr @malloc(%s %%tot)\n"
                   "  %%c1 = call ptr @memcpy(ptr %%buf, ptr %%a, %s %%la)\n"
                   "  %%end = getelementptr i8, ptr %%buf, %s %%la\n"
                   "  %%lb1 = add %s %%lb, 1\n"
                   "  %%c2 = call ptr @memcpy(ptr %%end, ptr %%b, %s %%lb1)\n"
                   "  ret ptr %%buf\n}\n\n",
                   U, U, U, U, U, U, U, U, U));
}

static void ll_emit_isws(ll_t *ll)
{
    sb_puts(ll->hg, "define internal i1 @salam_ll_isws(i8 %c) nounwind willreturn "
                    "memory(none) {\n"
                    "entry:\n"
                    "  %s32 = icmp eq i8 %c, 32\n"
                    "  %s9 = icmp eq i8 %c, 9\n"
                    "  %s10 = icmp eq i8 %c, 10\n"
                    "  %s13 = icmp eq i8 %c, 13\n"
                    "  %o1 = or i1 %s32, %s9\n"
                    "  %o2 = or i1 %o1, %s10\n"
                    "  %o3 = or i1 %o2, %s13\n"
                    "  ret i1 %o3\n}\n\n");
}

static void ll_emit_trim(ll_t *ll)
{
    ll_need(ll, LL_H_ISWS);
    const char *U = ll->usize;
    sb_puts(ll->hg, ll_fmt(ll,
                           "define internal noalias ptr @salam_ll_trim(ptr %%s) nounwind "
                           "willreturn nofree {\n"
                           "entry:\n"
                           "  %%n = call %s @strlen(ptr %%s)\n"
                           "  br label %%f\n"
                           "f:\n"
                           "  %%a = phi %s [ 0, %%entry ], [ %%an, %%fbody ]\n"
                           "  %%c1 = icmp ult %s %%a, %%n\n"
                           "  br i1 %%c1, label %%fchk, label %%b\n"
                           "fchk:\n"
                           "  %%p1 = getelementptr i8, ptr %%s, %s %%a\n"
                           "  %%ch1 = load i8, ptr %%p1\n"
                           "  %%w1 = call i1 @salam_ll_isws(i8 %%ch1)\n"
                           "  br i1 %%w1, label %%fbody, label %%b\n"
                           "fbody:\n"
                           "  %%an = add %s %%a, 1\n"
                           "  br label %%f\n"
                           "b:\n"
                           "  br label %%r\n"
                           "r:\n"
                           "  %%e = phi %s [ %%n, %%b ], [ %%en, %%rbody ]\n"
                           "  %%c2 = icmp ugt %s %%e, %%a\n"
                           "  br i1 %%c2, label %%rchk, label %%done\n"
                           "rchk:\n"
                           "  %%em1 = sub %s %%e, 1\n"
                           "  %%p2 = getelementptr i8, ptr %%s, %s %%em1\n"
                           "  %%ch2 = load i8, ptr %%p2\n"
                           "  %%w2 = call i1 @salam_ll_isws(i8 %%ch2)\n"
                           "  br i1 %%w2, label %%rbody, label %%done\n"
                           "rbody:\n"
                           "  %%en = sub %s %%e, 1\n"
                           "  br label %%r\n"
                           "done:\n"
                           "  %%len = sub %s %%e, %%a\n"
                           "  %%tot = add %s %%len, 1\n"
                           "  %%buf = call ptr @malloc(%s %%tot)\n"
                           "  %%src = getelementptr i8, ptr %%s, %s %%a\n"
                           "  %%cp = call ptr @memcpy(ptr %%buf, ptr %%src, %s %%len)\n"
                           "  %%endp = getelementptr i8, ptr %%buf, %s %%len\n"
                           "  store i8 0, ptr %%endp\n"
                           "  ret ptr %%buf\n}\n\n",
                           U, U, U, U, U, U, U, U, U, U, U, U, U, U, U, U));
}

static void ll_emit_strhash(ll_t *ll)
{
    sb_puts(ll->hg, "define internal i64 @salam_ll_strhash(ptr %s) nounwind willreturn "
                    "memory(argmem: read) {\n"
                    "entry:\n  br label %loop\n"
                    "loop:\n"
                    "  %j = phi i64 [ 0, %entry ], [ %jn, %body ]\n"
                    "  %h = phi i64 [ 14695981039346656037, %entry ], [ %hn, %body ]\n"
                    "  %p = getelementptr i8, ptr %s, i64 %j\n"
                    "  %c8 = load i8, ptr %p\n"
                    "  %z = icmp eq i8 %c8, 0\n"
                    "  br i1 %z, label %done, label %body\n"
                    "body:\n"
                    "  %c = zext i8 %c8 to i64\n"
                    "  %hm = mul i64 %h, 1099511628211\n"
                    "  %hn = add i64 %hm, %c\n"
                    "  %jn = add i64 %j, 1\n"
                    "  br label %loop\n"
                    "done:\n  ret i64 %h\n}\n\n");
}

static void ll_emit_inthash(ll_t *ll)
{
    sb_puts(ll->hg, "define internal i64 @salam_ll_inthash(i64 %x) nounwind willreturn "
                    "memory(none) {\n"
                    "entry:\n"
                    "  %h0 = add i64 %x, 14695981039346656037\n"
                    "  %h1 = mul i64 %h0, 1099511628211\n"
                    "  %h2 = add i64 %h1, 1099511628211\n"
                    "  %h3 = mul i64 %h2, 1099511628211\n"
                    "  ret i64 %h3\n}\n\n");
}

static void ll_emit_i64str(ll_t *ll)
{
    const char *U = ll->usize;
    sb_puts(ll->hg,
            "@.sfmt.lld = private unnamed_addr constant [5 x i8] c\"%lld\\00\"\n");
    sb_puts(ll->hg,
            ll_fmt(ll,
                   "define internal noalias ptr @salam_ll_i64str(i64 %%v) nounwind "
                   "willreturn nofree {\n"
                   "entry:\n"
                   "  %%b = call ptr @malloc(%s 32)\n"
                   "  %%r = call i32 (ptr, %s, ptr, ...) @snprintf(ptr %%b, %s 32, "
                   "ptr @.sfmt.lld, i64 %%v)\n"
                   "  ret ptr %%b\n}\n\n",
                   U, U, U));
}

static void ll_emit_u64str(ll_t *ll)
{
    const char *U = ll->usize;
    sb_puts(ll->hg,
            "@.sfmt.llu = private unnamed_addr constant [5 x i8] c\"%llu\\00\"\n");
    sb_puts(ll->hg,
            ll_fmt(ll,
                   "define internal noalias ptr @salam_ll_u64str(i64 %%v) nounwind "
                   "willreturn nofree {\n"
                   "entry:\n"
                   "  %%b = call ptr @malloc(%s 32)\n"
                   "  %%r = call i32 (ptr, %s, ptr, ...) @snprintf(ptr %%b, %s 32, "
                   "ptr @.sfmt.llu, i64 %%v)\n"
                   "  ret ptr %%b\n}\n\n",
                   U, U, U));
}

static void ll_emit_f64str(ll_t *ll)
{
    const char *U = ll->usize;
    sb_puts(ll->hg, "@.sfmt.g = private unnamed_addr constant [3 x i8] c\"%g\\00\"\n");
    sb_puts(ll->hg,
            ll_fmt(ll,
                   "define internal noalias ptr @salam_ll_f64str(double %%v) nounwind "
                   "willreturn nofree {\n"
                   "entry:\n"
                   "  %%b = call ptr @malloc(%s 32)\n"
                   "  %%r = call i32 (ptr, %s, ptr, ...) @snprintf(ptr %%b, %s 32, "
                   "ptr @.sfmt.g, double %%v)\n"
                   "  ret ptr %%b\n}\n\n",
                   U, U, U));
}

static void ll_emit_charstr(ll_t *ll)
{
    sb_puts(ll->hg,
            ll_fmt(ll,
                   "define internal noalias ptr @salam_ll_charstr(i32 %%c) nounwind "
                   "willreturn nofree {\n"
                   "entry:\n"
                   "  %%b = call ptr @malloc(%s 2)\n"
                   "  %%t = trunc i32 %%c to i8\n"
                   "  store i8 %%t, ptr %%b\n"
                   "  %%p1 = getelementptr i8, ptr %%b, i64 1\n"
                   "  store i8 0, ptr %%p1\n"
                   "  ret ptr %%b\n}\n\n",
                   ll->usize));
}

static void ll_emit_outbuf(ll_t *ll)
{
    sb_puts(ll->hg,
            "@salam_ob = internal global [65536 x i8] zeroinitializer\n"
            "@salam_obn = internal global i64 0\n"
            "@stdout = external global ptr\n"
            "declare i32 @fflush(ptr) nounwind\n"
            "declare void @llvm.memcpy.p0.p0.i64(ptr, ptr, i64, i1) nounwind\n"
            "define internal void @salam_out_flush() nounwind {\n"
            "entry:\n"
            "  %n = load i64, ptr @salam_obn\n"
            "  %z = icmp eq i64 %n, 0\n"
            "  br i1 %z, label %done, label %do\n"
            "do:\n"
            "  %r = call i64 @write(i32 1, ptr @salam_ob, i64 %n)\n"
            "  store i64 0, ptr @salam_obn\n"
            "  br label %done\n"
            "done:\n"
            "  ret void\n"
            "}\n"
            "define internal void @salam_out_write(ptr %s, i64 %n) nounwind {\n"
            "entry:\n"
            "  %obn = load i64, ptr @salam_obn\n"
            "  %sum = add i64 %obn, %n\n"
            "  %over = icmp ugt i64 %sum, 65536\n"
            "  br i1 %over, label %flush, label %chk\n"
            "flush:\n"
            "  call void @salam_out_flush()\n"
            "  br label %chk\n"
            "chk:\n"
            "  %big = icmp uge i64 %n, 65536\n"
            "  br i1 %big, label %direct, label %copy\n"
            "direct:\n"
            "  %rd = call i64 @write(i32 1, ptr %s, i64 %n)\n"
            "  ret void\n"
            "copy:\n"
            "  %obn2 = load i64, ptr @salam_obn\n"
            "  %dst = getelementptr inbounds [65536 x i8], ptr @salam_ob, i64 0, i64 "
            "%obn2\n"
            "  call void @llvm.memcpy.p0.p0.i64(ptr %dst, ptr %s, i64 %n, i1 false)\n"
            "  %newn = add i64 %obn2, %n\n"
            "  store i64 %newn, ptr @salam_obn\n"
            "  ret void\n"
            "}\n"
            "@llvm.global_dtors = appending global [1 x { i32, ptr, ptr }] "
            "[{ i32, ptr, ptr } { i32 65535, ptr @salam_out_flush, ptr null }]\n\n");
}

static void (*const LL_HELPER_EMIT[LL_H_COUNT])(ll_t *) = {
    ll_emit_substr,  ll_emit_strcat,  ll_emit_isws,   ll_emit_trim,
    ll_emit_strhash, ll_emit_inthash, ll_emit_i64str, ll_emit_u64str,
    ll_emit_f64str,  ll_emit_charstr, ll_emit_outbuf,
};

void ll_need(ll_t *ll, ll_helper_t which)
{
    if (ll->hneed[which]) return;
    ll->hneed[which] = true;
    LL_HELPER_EMIT[which](ll);
}
static void ll_toplevel(ll_t *ll, ast_node_t *d)
{
    switch (d->kind) {
    case AST_FUNC_DEF:
        if (d->is_extern && !d->a) return;
        if (d->typarams.len > 0) return;
        ll_function(ll, d, NULL);
        break;
    case AST_STRUCT_DEF: {
        if (d->typarams.len > 0) break;
        symbol_t *ssym = ll_struct_sym(ll, d->name);
        {
            size_t i = 0;
            for (; i < d->list.len; i++) {
                ast_node_t *m = (ast_node_t *)d->list.data[i];
                if (m->kind == AST_FUNC_DEF) ll_function(ll, m, ssym);
            }
        }
        break;
    }
    case AST_ENUM_DEF:
    case AST_CONST_DECL:
    case AST_VAR_DECL:
    case AST_INTERFACE_DEF:
    case AST_IMPL_DEF:
    case AST_IMPORT:
    case AST_LINK:
    case AST_TYPE_ALIAS:
        break;
    default:
        ll_error(ll, d, "top-level %s (layout/unsupported)", ast_kind_name(d->kind));
        break;
    }
}

void codegen_llvm_options_default(codegen_llvm_options_t *o)
{
    if (o) memset(o, 0, sizeof *o);
}

const char *codegen_llvm_error(const llvm_output_t *out)
{
    return out ? out->error : NULL;
}

llvm_output_t *codegen_llvm_run_opts(arena_t *a, logger_t *log, ast_node_t *program,
                                     sema_result_t *sem, const char *module,
                                     const char *entry,
                                     const codegen_llvm_options_t *opts,
                                     const char *src_path)
{
    ll_t ll;
    memset(&ll, 0, sizeof ll);
    ll.a = a;
    ll.log = log;
    ll.sem = sem;
    ll.module = module;
    ll.entry = (entry && entry[0]) ? entry : "main";
    ll.ok = true;
    ll.debug = opts && opts->debug_info;
    ll.triple = (opts && opts->target_triple && opts->target_triple[0])
                    ? opts->target_triple
                    : NULL;
    ll.ptr_bits = ll_target_ptr_bits(ll.triple);
    ll.usize = ll.ptr_bits == 32 ? "i32" : "i64";
    ll.optsize =
        opts && (opts->opt_level == LLVM_OPT_OS || opts->opt_level == LLVM_OPT_OZ);
    ll.minsize = opts && opts->opt_level == LLVM_OPT_OZ;
    ll.single_threaded =
        salam_module_single_threaded(program) && !ll_target_is_windows(ll.triple);
    vec_init(&ll.locals);
    vec_init(&ll.strings);
    vec_init(&ll.defers);
    vec_init(&ll.globals);
    vec_init(&ll.gdefer);
    vec_init(&ll.extern_names);
    vec_init(&ll.emitted);
    sb_t g, meta, hg;
    sb_init(&g);
    sb_init(&meta);
    sb_init(&hg);
    ll.g = &g;
    ll.b = &g;
    ll.meta = &meta;
    ll.hg = &hg;
    sb_puts(&g, ll_fmt(&ll, "; ModuleID = '%s'\n", module));
    sb_puts(&g, ll_fmt(&ll, "source_filename = \"%s.salam\"\n\n", module));
    if (ll.triple) sb_puts(&g, ll_fmt(&ll, "target triple = \"%s\"\n", ll.triple));
    {
        const char *dl = ll_datalayout(ll.triple);
        if (dl) sb_puts(&g, ll_fmt(&ll, "target datalayout = \"%s\"\n", dl));
    }
    sb_puts(&g, "\n");
    ll_emit_prologue(&ll);
    ll_emit_struct_types(&ll, program);
    /* Some packages (e.g. collections, whose generic Vector<T>.get/.set do
     * their own bounds checking) declare salam_panic/salam_idx as a local,
     * bodyless `extern:` stub instead of importing core - that stub resolves
     * fine for the C backend (real symbols are found at link time across
     * separately-compiled translation units) but leaves LLVM, which compiles
     * one function body per resolved declaration on demand, with only a
     * `declare` and no `define` when nothing else pulls in core's real
     * implementation. Force-compile core's actual definitions - before any
     * package's own bodyless stub can be seen and turned into a competing
     * bare `declare` - so every program that can panic on an out-of-bounds
     * access links cleanly. */
    {
        static const char *const rt_essentials[] = {"salam_panic", "salam_idx", NULL};
        symbol_t *core_pk = NULL;
        size_t p = 0;
        for (; p < ll.sem->packages.len; p++) {
            symbol_t *pk = (symbol_t *)ll.sem->packages.data[p];
            if (pk && pk->kind == SYM_PACKAGE && pk->name && !strcmp(pk->name, "core")) {
                core_pk = pk;
                break;
            }
        }
        if (core_pk && core_pk->members) {
            size_t i = 0;
            for (; rt_essentials[i]; i++) {
                symbol_t *fs = scope_lookup_local(core_pk->members, rt_essentials[i]);
                if (!fs || fs->kind != SYM_FUNC || fs->overloads.len == 0) continue;
                func_sig_t *sig = (func_sig_t *)fs->overloads.data[0];
                if (sig->decl && sig->decl->is_extern && !sig->decl->a) continue;
                ll_ensure_fn(&ll, sig->decl, NULL, core_pk->members);
            }
        }
    }
    ll_emit_externs(&ll);
    ll_emit_globals(&ll, program);
    if (ll.debug) ll_debug_init(&ll, src_path);
    ll_emit_impls(&ll);
    ll_emit_packages(&ll);
    LOG_I(log, PH_CODEGEN, "generating LLVM IR for module '%s'%s", module,
          ll.debug ? i18n_tr(" (+debug info)") : "");
    {
        size_t i = 0;
        for (; i < program->list.len; i++)
            ll_toplevel(&ll, (ast_node_t *)program->list.data[i]);
    }
    if (ll.debug) ll_debug_finalize(&ll);
    if (hg.len > 0) {
        sb_puts(&g, "; on-demand runtime helpers\n");
        sb_puts(&g, sb_cstr(&hg));
    }
    llvm_output_t *out = (llvm_output_t *)arena_alloc(a, sizeof *out);
    out->module = module;
    out->ll_src = arena_strdup(a, sb_cstr(&g));
    out->ok = ll.ok;
    out->error = ll.first_error;
    sb_free(&g);
    sb_free(&meta);
    sb_free(&hg);
    LOG_I(log, PH_CODEGEN, "generated %zu bytes of LLVM IR%s", strlen(out->ll_src),
          ll.ok ? "" : i18n_tr(" (with unsupported constructs)"));
    return out;
}

llvm_output_t *codegen_llvm_run(arena_t *a, logger_t *log, ast_node_t *program,
                                sema_result_t *sem, const char *module, const char *entry)
{
    return codegen_llvm_run_opts(a, log, program, sem, module, entry, NULL, NULL);
}
