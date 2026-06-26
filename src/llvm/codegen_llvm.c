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

static const char *LL_PROLOGUE =
    "; Salam -> LLVM IR (textual). Target-independent: compile with\n"
    ";   clang prog.ll -o prog      (native)\n"
    ";   clang -target wasm32 ...   (WebAssembly)\n"
    "declare i32 @printf(ptr, ...)\n"
    "declare i32 @dprintf(i32, ptr, ...)\n"
    "declare i64 @strlen(ptr)\n"
    "declare i32 @strcmp(ptr, ptr)\n"
    "declare ptr @malloc(i64)\n"
    "declare ptr @realloc(ptr, i64)\n"
    "declare void @free(ptr)\n"
    "declare ptr @memcpy(ptr, ptr, i64)\n"
    "declare ptr @memmove(ptr, ptr, i64)\n"
    "declare void @abort()\n"
    "declare void @exit(i32)\n"
    "declare i32 @snprintf(ptr, i64, ptr, ...)\n"
    "declare i64 @strtol(ptr, ptr, i32)\n"
    "declare double @strtod(ptr, ptr)\n"
    "declare ptr @strstr(ptr, ptr)\n"
    "declare double @llvm.pow.f64(double, double)\n"
    "\n"
    "; str.substr(start,len): a fresh NUL-terminated slice\n"
    "define ptr @salam_ll_substr(ptr %s, i32 %start, i32 %len) {\n"
    "entry:\n"
    "  %l = sext i32 %len to i64\n"
    "  %t = add i64 %l, 1\n"
    "  %buf = call ptr @malloc(i64 %t)\n"
    "  %so = sext i32 %start to i64\n"
    "  %src = getelementptr i8, ptr %s, i64 %so\n"
    "  %c = call ptr @memcpy(ptr %buf, ptr %src, i64 %l)\n"
    "  %e = getelementptr i8, ptr %buf, i64 %l\n"
    "  store i8 0, ptr %e\n"
    "  ret ptr %buf\n"
    "}\n"
    "\n"
    "@.panicfmt = private unnamed_addr constant [17 x i8] c\"salam panic: %s\\0A\\00\"\n"
    "\n"
    "; `dyn Iface` fat pointer: { data, vtable } (vtable = [N x ptr] of methods)\n"
    "%dyn = type { ptr, ptr }\n"
    "\n"
    "; hash intrinsics (match std/mem salam_str_hash / salam_hash_int, FNV-style)\n"
    "define i64 @salam_ll_strhash(ptr %s) {\n"
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
    "define i64 @salam_ll_inthash(i64 %x) {\n"
    "entry:\n"
    "  %h0 = add i64 %x, 14695981039346656037\n"
    "  %h1 = mul i64 %h0, 1099511628211\n"
    "  %h2 = add i64 %h1, 1099511628211\n"
    "  %h3 = mul i64 %h2, 1099511628211\n"
    "  ret i64 %h3\n}\n"
    "\n"
    "@.sfmt.lld = private unnamed_addr constant [5 x i8] c\"%lld\\00\"\n"
    "@.sfmt.llu = private unnamed_addr constant [5 x i8] c\"%llu\\00\"\n"
    "@.sfmt.g   = private unnamed_addr constant [3 x i8] c\"%g\\00\"\n"
    "\n"
    "; number/char -> freshly heap-allocated decimal string (for `str + x`)\n"
    "define ptr @salam_ll_i64str(i64 %v) {\n"
    "entry:\n"
    "  %b = call ptr @malloc(i64 32)\n"
    "  %r = call i32 (ptr, i64, ptr, ...) @snprintf(ptr %b, i64 32, ptr @.sfmt.lld, i64 %v)\n"
    "  ret ptr %b\n"
    "}\n"
    "define ptr @salam_ll_u64str(i64 %v) {\n"
    "entry:\n"
    "  %b = call ptr @malloc(i64 32)\n"
    "  %r = call i32 (ptr, i64, ptr, ...) @snprintf(ptr %b, i64 32, ptr @.sfmt.llu, i64 %v)\n"
    "  ret ptr %b\n"
    "}\n"
    "define ptr @salam_ll_f64str(double %v) {\n"
    "entry:\n"
    "  %b = call ptr @malloc(i64 32)\n"
    "  %r = call i32 (ptr, i64, ptr, ...) @snprintf(ptr %b, i64 32, ptr @.sfmt.g, double %v)\n"
    "  ret ptr %b\n"
    "}\n"
    "define ptr @salam_ll_charstr(i32 %c) {\n"
    "entry:\n"
    "  %b = call ptr @malloc(i64 2)\n"
    "  %t = trunc i32 %c to i8\n"
    "  store i8 %t, ptr %b\n"
    "  %p1 = getelementptr i8, ptr %b, i64 1\n"
    "  store i8 0, ptr %p1\n"
    "  ret ptr %b\n"
    "}\n"
    "\n"
    "; heap-concatenate two NUL-terminated strings\n"
    "define ptr @salam_ll_strcat(ptr %a, ptr %b) {\n"
    "entry:\n"
    "  %la = call i64 @strlen(ptr %a)\n"
    "  %lb = call i64 @strlen(ptr %b)\n"
    "  %sum = add i64 %la, %lb\n"
    "  %tot = add i64 %sum, 1\n"
    "  %buf = call ptr @malloc(i64 %tot)\n"
    "  %c1 = call ptr @memcpy(ptr %buf, ptr %a, i64 %la)\n"
    "  %end = getelementptr i8, ptr %buf, i64 %la\n"
    "  %lb1 = add i64 %lb, 1\n"
    "  %c2 = call ptr @memcpy(ptr %end, ptr %b, i64 %lb1)\n"
    "  ret ptr %buf\n"
    "}\n\n";
static void ll_toplevel(ll_t *ll, ast_node_t *d)
{
    switch (d->kind) {
        case AST_FUNC_DEF:
            if (d->is_extern && !d->a) return;   
            if (d->typarams.len > 0)   return;   
            ll_function(ll, d, NULL);
            break;
        case AST_STRUCT_DEF: {
            if (d->typarams.len > 0) break;      
            symbol_t *ssym = ll_struct_sym(ll, d->name);   
            { size_t i = 0; for (; i < d->list.len; i++) {
                ast_node_t *m = (ast_node_t *)d->list.data[i];
                if (m->kind == AST_FUNC_DEF) ll_function(ll, m, ssym);
            } }
            break;                               
        }
        case AST_ENUM_DEF:                        
        case AST_CONST_DECL: case AST_VAR_DECL:   
        case AST_INTERFACE_DEF:                   
        case AST_IMPL_DEF:                        
        case AST_IMPORT: case AST_LINK: case AST_TYPE_ALIAS:
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
    ll_t ll; memset(&ll, 0, sizeof ll);
    ll.a = a; ll.log = log; ll.sem = sem; ll.module = module;
    ll.entry = (entry && entry[0]) ? entry : "main";
    ll.ok = true;
    ll.debug = opts && opts->debug_info;
    vec_init(&ll.locals); vec_init(&ll.strings); vec_init(&ll.defers);
    vec_init(&ll.globals); vec_init(&ll.gdefer); vec_init(&ll.extern_names);
    vec_init(&ll.emitted);
    sb_t g, meta; sb_init(&g); sb_init(&meta);
    ll.g = &g; ll.b = &g; ll.meta = &meta;
    sb_puts(&g, ll_fmt(&ll, "; ModuleID = '%s'\n", module));
    sb_puts(&g, ll_fmt(&ll, "source_filename = \"%s.salam\"\n\n", module));
    sb_puts(&g, LL_PROLOGUE);
    ll_emit_struct_types(&ll, program);   
    ll_emit_externs(&ll);                 
    ll_emit_globals(&ll, program);
    if (ll.debug) ll_debug_init(&ll, src_path);
    ll_emit_impls(&ll);                   
    ll_emit_packages(&ll);                
    LOG_I(log, PH_CODEGEN, "generating LLVM IR for module '%s'%s",
          module, ll.debug ? i18n_tr(" (+debug info)") : "");
    { size_t i = 0; for (; i < program->list.len; i++)
        ll_toplevel(&ll, (ast_node_t *)program->list.data[i]); }
    if (ll.debug) ll_debug_finalize(&ll);
    llvm_output_t *out = (llvm_output_t *)arena_alloc(a, sizeof *out);
    out->module = module;
    out->ll_src = arena_strdup(a, sb_cstr(&g));
    out->ok = ll.ok;
    out->error = ll.first_error;
    sb_free(&g); sb_free(&meta);
    LOG_I(log, PH_CODEGEN, "generated %zu bytes of LLVM IR%s",
          strlen(out->ll_src), ll.ok ? "" : i18n_tr(" (with unsupported constructs)"));
    return out;
}
llvm_output_t *codegen_llvm_run(arena_t *a, logger_t *log, ast_node_t *program,
                                sema_result_t *sem, const char *module,
                                const char *entry)
{
    return codegen_llvm_run_opts(a, log, program, sem, module, entry, NULL, NULL);
}
