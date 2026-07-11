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
#include "i18n/i18n.h"

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

    sb_puts(g, "define internal noalias ptr @salam_ll_substr(ptr %s, i32 %start, i32 "
               "%len) nounwind {\nentry:\n");
    const char *L = pl_widen(ll, g, "%len", "%l");
    const char *SO = pl_widen(ll, g, "%start", "%so");
    sb_puts(g, ll_fmt(ll, "  %%t = add %s %s, 1\n", U, L));
    sb_puts(g, ll_fmt(ll, "  %%buf = call ptr @malloc(%s %%t)\n", U));
    sb_puts(g, ll_fmt(ll, "  %%src = getelementptr i8, ptr %%s, %s %s\n", U, SO));
    sb_puts(g,
            ll_fmt(ll, "  %%c = call ptr @memcpy(ptr %%buf, ptr %%src, %s %s)\n", U, L));
    sb_puts(g, ll_fmt(ll, "  %%e = getelementptr i8, ptr %%buf, %s %s\n", U, L));
    sb_puts(g, "  store i8 0, ptr %e\n  ret ptr %buf\n}\n\n");

    sb_puts(g, "@.panicfmt = private unnamed_addr constant [17 x i8] c\"salam panic: "
               "%s\\0A\\00\"\n\n");

    sb_puts(
        g,
        "; `dyn Iface` fat pointer: { data, vtable } (vtable = [N x ptr] of methods)\n");
    sb_puts(g, "%dyn = type { ptr, ptr }\n\n");

    sb_puts(g, "define internal i64 @salam_ll_strhash(ptr %s) nounwind willreturn "
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
               "done:\n  ret i64 %h\n}\n"
               "define internal i64 @salam_ll_inthash(i64 %x) nounwind willreturn "
               "memory(none) {\n"
               "entry:\n"
               "  %h0 = add i64 %x, 14695981039346656037\n"
               "  %h1 = mul i64 %h0, 1099511628211\n"
               "  %h2 = add i64 %h1, 1099511628211\n"
               "  %h3 = mul i64 %h2, 1099511628211\n"
               "  ret i64 %h3\n}\n\n");

    sb_puts(g, "@.sfmt.lld = private unnamed_addr constant [5 x i8] c\"%lld\\00\"\n");
    sb_puts(g, "@.sfmt.llu = private unnamed_addr constant [5 x i8] c\"%llu\\00\"\n");
    sb_puts(g, "@.sfmt.g   = private unnamed_addr constant [3 x i8] c\"%g\\00\"\n\n");

    sb_puts(g, ll_fmt(ll,
                      "define internal noalias ptr @salam_ll_i64str(i64 %%v) nounwind {\n"
                      "entry:\n"
                      "  %%b = call ptr @malloc(%s 32)\n"
                      "  %%r = call i32 (ptr, %s, ptr, ...) @snprintf(ptr %%b, %s 32, "
                      "ptr @.sfmt.lld, i64 %%v)\n"
                      "  ret ptr %%b\n}\n",
                      U, U, U));
    sb_puts(g, ll_fmt(ll,
                      "define internal noalias ptr @salam_ll_u64str(i64 %%v) nounwind {\n"
                      "entry:\n"
                      "  %%b = call ptr @malloc(%s 32)\n"
                      "  %%r = call i32 (ptr, %s, ptr, ...) @snprintf(ptr %%b, %s 32, "
                      "ptr @.sfmt.llu, i64 %%v)\n"
                      "  ret ptr %%b\n}\n",
                      U, U, U));
    sb_puts(g, ll_fmt(ll,
                      "define internal noalias ptr @salam_ll_f64str(double %%v) nounwind "
                      "{\n"
                      "entry:\n"
                      "  %%b = call ptr @malloc(%s 32)\n"
                      "  %%r = call i32 (ptr, %s, ptr, ...) @snprintf(ptr %%b, %s 32, "
                      "ptr @.sfmt.g, double %%v)\n"
                      "  ret ptr %%b\n}\n",
                      U, U, U));
    sb_puts(g, ll_fmt(ll,
                      "define internal noalias ptr @salam_ll_charstr(i32 %%c) nounwind "
                      "{\n"
                      "entry:\n"
                      "  %%b = call ptr @malloc(%s 2)\n"
                      "  %%t = trunc i32 %%c to i8\n"
                      "  store i8 %%t, ptr %%b\n"
                      "  %%p1 = getelementptr i8, ptr %%b, i64 1\n"
                      "  store i8 0, ptr %%p1\n"
                      "  ret ptr %%b\n}\n\n",
                      U));

    sb_puts(g, "define internal i1 @salam_ll_isws(i8 %c) nounwind willreturn "
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

    sb_puts(g, ll_fmt(ll,
                      "define internal noalias ptr @salam_ll_trim(ptr %%s) nounwind {\n"
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

    sb_puts(g, ll_fmt(ll,
                      "define internal noalias ptr @salam_ll_strcat(ptr %%a, ptr %%b) "
                      "nounwind {\n"
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
    vec_init(&ll.locals);
    vec_init(&ll.strings);
    vec_init(&ll.defers);
    vec_init(&ll.globals);
    vec_init(&ll.gdefer);
    vec_init(&ll.extern_names);
    vec_init(&ll.emitted);
    sb_t g, meta;
    sb_init(&g);
    sb_init(&meta);
    ll.g = &g;
    ll.b = &g;
    ll.meta = &meta;
    sb_puts(&g, ll_fmt(&ll, "; ModuleID = '%s'\n", module));
    sb_puts(&g, ll_fmt(&ll, "source_filename = \"%s.salam\"\n\n", module));
    if (ll.triple) sb_puts(&g, ll_fmt(&ll, "target triple = \"%s\"\n\n", ll.triple));
    ll_emit_prologue(&ll);
    ll_emit_struct_types(&ll, program);
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
    llvm_output_t *out = (llvm_output_t *)arena_alloc(a, sizeof *out);
    out->module = module;
    out->ll_src = arena_strdup(a, sb_cstr(&g));
    out->ok = ll.ok;
    out->error = ll.first_error;
    sb_free(&g);
    sb_free(&meta);
    LOG_I(log, PH_CODEGEN, "generated %zu bytes of LLVM IR%s", strlen(out->ll_src),
          ll.ok ? "" : i18n_tr(" (with unsupported constructs)"));
    return out;
}

llvm_output_t *codegen_llvm_run(arena_t *a, logger_t *log, ast_node_t *program,
                                sema_result_t *sem, const char *module, const char *entry)
{
    return codegen_llvm_run_opts(a, log, program, sem, module, entry, NULL, NULL);
}
