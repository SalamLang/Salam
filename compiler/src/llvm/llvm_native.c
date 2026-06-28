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
#include "core/sal_format.h"
#include "llvm/llvm_native.h"
#include "core/sb.h"
#include "semantic/sema.h"
#include "i18n/i18n.h"
#include <stdlib.h>

#ifndef SALAM_HAVE_LLVM
int salam_llvm_native_available(void) { return 0; }

int salam_llvm_native(logger_t *log, const char *ll_path,
                      const codegen_llvm_options_t *opts)
{
    (void)log; (void)ll_path; (void)opts;
    return -1;
}

#else /* SALAM_HAVE_LLVM */

#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Target.h>
#include <llvm-c/TargetMachine.h>
#include <llvm-c/BitWriter.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Error.h>
#include <llvm-c/Orc.h>
#include <llvm-c/LLJIT.h>
#include <llvm-c/Transforms/PassBuilder.h>

int salam_llvm_native_available(void) { return 1; }

static LLVMCodeGenOptLevel map_cg_level(llvm_opt_level_t l)
{
    switch (l) {
        case LLVM_OPT_O0: return LLVMCodeGenLevelNone;
        case LLVM_OPT_O1: return LLVMCodeGenLevelLess;
        case LLVM_OPT_O2: return LLVMCodeGenLevelDefault;
        case LLVM_OPT_O3: return LLVMCodeGenLevelAggressive;
        case LLVM_OPT_OS: return LLVMCodeGenLevelDefault;
        case LLVM_OPT_OZ: return LLVMCodeGenLevelDefault;
        default:          return LLVMCodeGenLevelNone;
    }
}

static const char *map_pass_pipeline(llvm_opt_level_t l)
{
    switch (l) {
        case LLVM_OPT_O1: return "default<O1>";
        case LLVM_OPT_O2: return "default<O2>";
        case LLVM_OPT_O3: return "default<O3>";
        case LLVM_OPT_OS: return "default<Os>";
        case LLVM_OPT_OZ: return "default<Oz>";
        default:          return NULL;
    }
}

static void init_targets_once(void)
{
    static int done = 0;
    if (done) return;
    done = 1;
    LLVMInitializeAllTargetInfos();
    LLVMInitializeAllTargets();
    LLVMInitializeAllTargetMCs();
    LLVMInitializeAllAsmPrinters();
    LLVMInitializeAllAsmParsers();
}

static int parse_ir(logger_t *log, const char *ll_path,
                    LLVMContextRef *out_ctx, LLVMModuleRef *out_mod)
{
    LLVMMemoryBufferRef buf = NULL;
    char *err = NULL;
    if (LLVMCreateMemoryBufferWithContentsOfFile(ll_path, &buf, &err)) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), ll_path);
        if (err) LLVMDisposeMessage(err);
        return 2;
    }
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMModuleRef mod = NULL;
    if (LLVMParseIRInContext(ctx, buf, &mod, &err)) {
        LOG_E(log, PH_DRIVER, "LLVM IR parse error: %s", err ? err : "(unknown)");
        if (err) LLVMDisposeMessage(err);
        LLVMContextDispose(ctx);
        return 2;
    }
    *out_ctx = ctx;
    *out_mod = mod;
    return 0;
}

static int run_opt(logger_t *log, LLVMModuleRef mod, LLVMTargetMachineRef tm,
                   llvm_opt_level_t level)
{
    const char *pipeline = map_pass_pipeline(level);
    if (!pipeline) return 0;
    LLVMPassBuilderOptionsRef pbo = LLVMCreatePassBuilderOptions();
    LLVMErrorRef e = LLVMRunPasses(mod, pipeline, tm, pbo);
    LLVMDisposePassBuilderOptions(pbo);
    if (e) {
        char *msg = LLVMGetErrorMessage(e);
        LOG_W(log, PH_DRIVER, "LLVM optimization failed: %s", msg ? msg : "(unknown)");
        if (msg) LLVMDisposeErrorMessage(msg);
        return 1;
    }
    return 0;
}

static const char *resolve_linker(char *buf, size_t cap)
{
    if (salam_find_bundled_tool("gcc",   buf, cap)) return buf;
    if (salam_find_bundled_tool("clang", buf, cap)) return buf;
    if (salam_find_bundled_tool("cc",    buf, cap)) return buf;
    if (salam_find_bundled_tool("tcc",   buf, cap)) return buf;
#if defined(_WIN32)
    return "gcc";
#else
    return "cc";
#endif
}

static int link_executable(logger_t *log, const char *obj, const char *out)
{
    char cc[1200];
    const char *linker = resolve_linker(cc, sizeof cc);
    int is_tcc = (strstr(linker, "tcc") != NULL);
    sb_t cmd; sb_init(&cmd);
    sb_puts(&cmd, "\""); sb_puts(&cmd, linker); sb_puts(&cmd, "\" \"");
    sb_puts(&cmd, obj); sb_puts(&cmd, "\" -o \""); sb_puts(&cmd, out); sb_puts(&cmd, "\"");
#if defined(_WIN32)
    sb_puts(&cmd, is_tcc ? " -lmsvcrt" : " -lm");
#else
    (void)is_tcc;
    sb_puts(&cmd, " -lm");
#endif
    LOG_I(log, PH_DRIVER, "link: %s", sb_cstr(&cmd));
    int rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
    return rc == 0 ? 0 : 1;
}

static int orc_fail(logger_t *log, const char *what, LLVMErrorRef e)
{
    char *msg = LLVMGetErrorMessage(e);
    LOG_E(log, PH_DRIVER, "%s: %s", what, msg ? msg : "(unknown)");
    if (msg) LLVMDisposeErrorMessage(msg);
    return 1;
}

static void define_runtime_symbols(LLVMOrcLLJITRef jit, LLVMOrcJITDylibRef jd)
{
#define RT_FLAGS { LLVMJITSymbolGenericFlagsExported | LLVMJITSymbolGenericFlagsCallable, 0 }
#define RT_SYM(fn) { LLVMOrcLLJITMangleAndIntern(jit, #fn), \
                     { (LLVMOrcExecutorAddress)(size_t)&fn, RT_FLAGS } }
    LLVMOrcCSymbolMapPair syms[] = {
        RT_SYM(printf),  RT_SYM(snprintf), RT_SYM(strlen),  RT_SYM(strcmp),
        RT_SYM(malloc),  RT_SYM(realloc),  RT_SYM(free),    RT_SYM(memcpy),
        RT_SYM(memmove), RT_SYM(abort),    RT_SYM(exit),    RT_SYM(strtol),
        RT_SYM(strtod),  RT_SYM(strstr)
    };
    LLVMOrcJITDylibDefine(jd,
        LLVMOrcAbsoluteSymbols(syms, sizeof syms / sizeof syms[0]));
#if defined(_WIN32)
    {
        extern void __main(void);
        LLVMOrcCSymbolMapPair w[] = {
            { LLVMOrcLLJITMangleAndIntern(jit, "__main"),
              { (LLVMOrcExecutorAddress)(size_t)&__main, RT_FLAGS } }
        };
        LLVMOrcJITDylibDefine(jd, LLVMOrcAbsoluteSymbols(w, 1));
    }
#endif
#undef RT_SYM
#undef RT_FLAGS
}

static int jit_run_file(logger_t *log, const char *ll_path)
{
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

#if defined(SALAM_LLVM_VERSION_MAJOR) && SALAM_LLVM_VERSION_MAJOR >= 21
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMOrcThreadSafeContextRef tsctx =
        LLVMOrcCreateNewThreadSafeContextFromLLVMContext(ctx);
#else
    LLVMOrcThreadSafeContextRef tsctx = LLVMOrcCreateNewThreadSafeContext();
    LLVMContextRef ctx = LLVMOrcThreadSafeContextGetContext(tsctx);
#endif

    LLVMMemoryBufferRef buf = NULL;
    char *err = NULL;
    if (LLVMCreateMemoryBufferWithContentsOfFile(ll_path, &buf, &err)) {
        LOG_E(log, PH_DRIVER, i18n_tr("cannot read '%s'"), ll_path);
        if (err) LLVMDisposeMessage(err);
        LLVMOrcDisposeThreadSafeContext(tsctx);
        return 2;
    }
    LLVMModuleRef mod = NULL;
    if (LLVMParseIRInContext(ctx, buf, &mod, &err)) {
        LOG_E(log, PH_DRIVER, "LLVM IR parse error: %s", err ? err : "(unknown)");
        if (err) LLVMDisposeMessage(err);
        LLVMOrcDisposeThreadSafeContext(tsctx);
        return 2;
    }

    LLVMOrcThreadSafeModuleRef tsm = LLVMOrcCreateNewThreadSafeModule(mod, tsctx);
    LLVMOrcDisposeThreadSafeContext(tsctx);

    LLVMOrcLLJITRef jit = NULL;
    LLVMErrorRef e = LLVMOrcCreateLLJIT(&jit, NULL);
    if (e) { LLVMOrcDisposeThreadSafeModule(tsm); return orc_fail(log, "JIT init failed", e); }

    LLVMOrcJITDylibRef jd = LLVMOrcLLJITGetMainJITDylib(jit);

    LLVMOrcDefinitionGeneratorRef gen = NULL;
    e = LLVMOrcCreateDynamicLibrarySearchGeneratorForProcess(
            &gen, LLVMOrcLLJITGetGlobalPrefix(jit), NULL, NULL);
    if (e) { LLVMOrcDisposeLLJIT(jit); LLVMOrcDisposeThreadSafeModule(tsm);
             return orc_fail(log, "JIT symbol generator failed", e); }
    LLVMOrcJITDylibAddGenerator(jd, gen);
    define_runtime_symbols(jit, jd);

    e = LLVMOrcLLJITAddLLVMIRModule(jit, jd, tsm);
    if (e) { LLVMOrcDisposeLLJIT(jit); return orc_fail(log, "JIT add module failed", e); }

    LLVMOrcExecutorAddress addr = 0;
    e = LLVMOrcLLJITLookup(jit, &addr, "main");
    if (e) { LLVMOrcDisposeLLJIT(jit); return orc_fail(log, i18n_tr("no 'main' function to run"), e); }
    if (!addr) { LLVMOrcDisposeLLJIT(jit);
                 LOG_E(log, PH_DRIVER, i18n_tr("no 'main' function to run")); return 1; }

    int (*main_fn)(void) = (int (*)(void))(size_t)addr;
    int rc = main_fn();

    LLVMOrcDisposeLLJIT(jit);
    return rc;
}

int salam_llvm_native(logger_t *log, const char *ll_path,
                      const codegen_llvm_options_t *opts)
{
    init_targets_once();

    if (opts->output_mode == LLVM_OUT_JIT)
        return jit_run_file(log, ll_path);

    LLVMContextRef ctx = NULL;
    LLVMModuleRef mod = NULL;
    if (parse_ir(log, ll_path, &ctx, &mod) != 0) return 2;

    char *host_triple = LLVMGetDefaultTargetTriple();
    const char *triple = (opts->target_triple && opts->target_triple[0])
                         ? opts->target_triple : host_triple;
    LLVMSetTarget(mod, triple);

    int rc = 0;

    if (opts->verify_module) {
        char *verr = NULL;
        if (LLVMVerifyModule(mod, LLVMReturnStatusAction, &verr)) {
            LOG_E(log, PH_DRIVER, "LLVM verifier: %s", verr ? verr : "(invalid module)");
            if (verr) LLVMDisposeMessage(verr);
            rc = 1;
            goto cleanup;
        }
        if (verr) LLVMDisposeMessage(verr);
    }

    if (opts->output_mode == LLVM_OUT_IR) {
        goto cleanup;
    }

    {
        LLVMTargetRef target = NULL;
        char *terr = NULL;
        if (LLVMGetTargetFromTriple(triple, &target, &terr)) {
            LOG_E(log, PH_DRIVER, "LLVM target '%s': %s", triple,
                  terr ? terr : "(unavailable)");
            if (terr) LLVMDisposeMessage(terr);
            rc = 2; goto cleanup;
        }
        LLVMTargetMachineRef tm = LLVMCreateTargetMachine(
            target, triple, "", "", map_cg_level(opts->opt_level),
            LLVMRelocPIC, LLVMCodeModelDefault);
        if (!tm) {
            LOG_E(log, PH_DRIVER, "could not create LLVM target machine for %s", triple);
            rc = 2; goto cleanup;
        }

        run_opt(log, mod, tm, opts->opt_level);

        if (opts->output_mode == LLVM_OUT_BITCODE) {
            if (LLVMWriteBitcodeToFile(mod, opts->output_file) != 0) {
                LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), opts->output_file);
                rc = 2;
            }
        } else if (opts->output_mode == LLVM_OUT_ASM ||
                   opts->output_mode == LLVM_OUT_OBJ) {
            LLVMCodeGenFileType ft = (opts->output_mode == LLVM_OUT_ASM)
                                     ? LLVMAssemblyFile : LLVMObjectFile;
            char *eerr = NULL;
            if (LLVMTargetMachineEmitToFile(tm, mod,
                    opts->output_file, ft, &eerr)) {
                LOG_E(log, PH_DRIVER, "LLVM emit failed: %s", eerr ? eerr : "(unknown)");
                if (eerr) LLVMDisposeMessage(eerr);
                rc = 2;
            }
        } else if (opts->output_mode == LLVM_OUT_EXEC) {
            char objpath[1024];
            sal_snprintf(objpath, sizeof objpath, "%s.o", opts->output_file);
            char *eerr = NULL;
            if (LLVMTargetMachineEmitToFile(tm, mod, objpath,
                    LLVMObjectFile, &eerr)) {
                LOG_E(log, PH_DRIVER, "LLVM emit failed: %s", eerr ? eerr : "(unknown)");
                if (eerr) LLVMDisposeMessage(eerr);
                rc = 2;
            } else {
                rc = link_executable(log, objpath, opts->output_file);
                remove(objpath);
            }
        }

        LLVMDisposeTargetMachine(tm);
    }

cleanup:
    if (mod) LLVMDisposeModule(mod);
    if (ctx) LLVMContextDispose(ctx);
    LLVMDisposeMessage(host_triple);
    return rc;
}

#endif /* SALAM_HAVE_LLVM */
