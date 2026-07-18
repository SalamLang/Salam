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

#ifdef SALAM_HAVE_LLD
#  include "llvm/lld_link.h"
#  if !defined(_WIN32)
#    include <dirent.h>
#  endif
#endif

#if defined(SALAM_HAVE_EMBED_MUSL) || defined(SALAM_HAVE_EMBED_MUSL_AARCH64) ||          \
    defined(SALAM_HAVE_EMBED_MUSL_I686) || defined(SALAM_HAVE_EMBED_MUSL_ARM) ||         \
    defined(SALAM_HAVE_EMBED_MINGW)
#  include "driver/embed_sysroot.h"
#endif
#ifdef SALAM_HAVE_EMBED_MUSL /* x86_64 host musl (kept name for compatibility) */
extern const unsigned char salam_embed_musl[];
extern const unsigned char salam_embed_musl_end[];
#endif
#ifdef SALAM_HAVE_EMBED_MUSL_AARCH64
extern const unsigned char salam_embed_musl_aarch64[];
extern const unsigned char salam_embed_musl_aarch64_end[];
#endif
#ifdef SALAM_HAVE_EMBED_MUSL_I686
extern const unsigned char salam_embed_musl_i686[];
extern const unsigned char salam_embed_musl_i686_end[];
#endif
#ifdef SALAM_HAVE_EMBED_MUSL_ARM
extern const unsigned char salam_embed_musl_arm[];
extern const unsigned char salam_embed_musl_arm_end[];
#endif
#ifdef SALAM_HAVE_EMBED_MINGW
extern const unsigned char salam_embed_mingw[];
extern const unsigned char salam_embed_mingw_end[];
#endif

#ifndef SALAM_HAVE_LLVM
int salam_llvm_native_available(void)
{
    return 0;
}

int salam_llvm_native(logger_t *log, const char *ll_path,
                      const codegen_llvm_options_t *opts)
{
    (void)log;
    (void)ll_path;
    (void)opts;
    return -1;
}

#else

#  include <llvm-c/Core.h>
#  include <llvm-c/IRReader.h>
#  include <llvm-c/Target.h>
#  include <llvm-c/TargetMachine.h>
#  include <llvm-c/BitWriter.h>
#  include <llvm-c/Analysis.h>
#  include <llvm-c/Error.h>
#  include <llvm-c/Orc.h>
#  include <llvm-c/LLJIT.h>
#  include <llvm-c/Transforms/PassBuilder.h>

int salam_llvm_native_available(void)
{
    return 1;
}

static LLVMCodeGenOptLevel map_cg_level(llvm_opt_level_t l)
{
    switch (l) {
    case LLVM_OPT_O0:
        return LLVMCodeGenLevelNone;
    case LLVM_OPT_O1:
        return LLVMCodeGenLevelLess;
    case LLVM_OPT_O2:
        return LLVMCodeGenLevelDefault;
    case LLVM_OPT_O3:
        return LLVMCodeGenLevelAggressive;
    case LLVM_OPT_OS:
        return LLVMCodeGenLevelDefault;
    case LLVM_OPT_OZ:
        return LLVMCodeGenLevelDefault;
    default:
        return LLVMCodeGenLevelNone;
    }
}

static const char *map_pass_pipeline(llvm_opt_level_t l)
{
    switch (l) {
    case LLVM_OPT_O1:
        return "default<O1>";
    case LLVM_OPT_O2:
        return "default<O2>";
    case LLVM_OPT_O3:
        return "default<O3>";
    case LLVM_OPT_OS:
        return "default<Os>";
    case LLVM_OPT_OZ:
        return "default<Oz>";
    default:
        return NULL;
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

static int parse_ir(logger_t *log, const char *ll_path, LLVMContextRef *out_ctx,
                    LLVMModuleRef *out_mod)
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
    if (salam_find_bundled_tool("gcc", buf, cap)) return buf;
    if (salam_find_bundled_tool("clang", buf, cap)) return buf;
    if (salam_find_bundled_tool("cc", buf, cap)) return buf;
    if (salam_find_bundled_tool("tcc", buf, cap)) return buf;
#  if defined(_WIN32)
    return "gcc";
#  else
    return "cc";
#  endif
}

static int link_executable(logger_t *log, const char *obj, const char *out,
                           const codegen_llvm_options_t *opts)
{
    char cc[1200];
    const char *linker = resolve_linker(cc, sizeof cc);
    int is_tcc = (strstr(linker, "tcc") != NULL);
    sb_t cmd;
    sb_init(&cmd);
    sb_puts(&cmd, "\"");
    sb_puts(&cmd, linker);
    sb_puts(&cmd, "\" \"");
    sb_puts(&cmd, obj);
    sb_puts(&cmd, "\" -o \"");
    sb_puts(&cmd, out);
    sb_puts(&cmd, "\"");
    {
        int i = 0;
        for (; i < opts->nlink; i++) {
            const char *spec = opts->link_libs[i];
            if (!spec) continue;
            /* Shell-escape: `spec` comes from a `link "..."` directive and this
             * command is run via system(); an unescaped name would allow shell
             * injection (e.g. link "x; rm -rf /"). */
            if (spec[0] == '-' || strpbrk(spec, "/\\.") != NULL) {
                sb_puts(&cmd, " ");
                sb_put_shell_arg(&cmd, spec);
            } else {
                sb_puts(&cmd, " -l");
                sb_put_shell_arg(&cmd, spec);
            }
        }
    }
#  if defined(_WIN32)
    sb_puts(&cmd, is_tcc ? " -lmsvcrt" : " -lm");
#  else
    (void)is_tcc;
    sb_puts(&cmd, " -lm");
#  endif
    LOG_I(log, PH_DRIVER, "link: %s", sb_cstr(&cmd));
    int rc = system(sb_cstr(&cmd));
    sb_free(&cmd);
    return rc == 0 ? 0 : 1;
}

static int lld_can_link(const char *t)
{
#  ifdef SALAM_HAVE_LLD
    if (!t) return 0;
    {
        int win = (strstr(t, "windows") || strstr(t, "mingw") || strstr(t, "win32")) != 0;
        int gnu = (strstr(t, "gnu") || strstr(t, "mingw")) != 0;
        int musl = (strstr(t, "linux") && strstr(t, "musl")) != 0;
        return (win && gnu) || musl;
    }
#  else
    (void)t;
    return 0;
#  endif
}

#  ifdef SALAM_HAVE_LLD
static const char *mingw_arch(const char *t)
{
    if (strstr(t, "aarch64") || strstr(t, "arm64")) return "aarch64";
    if (strstr(t, "i686") || strstr(t, "i386") || strstr(t, "i586")) return "i686";
    return "x86_64";
}

static const char *mingw_emul(const char *t)
{
    if (strstr(t, "aarch64") || strstr(t, "arm64")) return "arm64pe";
    if (strstr(t, "i686") || strstr(t, "i386") || strstr(t, "i586")) return "i386pe";
    return "i386pep";
}

static int find_mingw_gccdir(const char *arch, char *out, size_t n)
{
#    if defined(_WIN32)
    (void)arch;
    (void)out;
    (void)n;
    return 0;
#    else
    char base[256];
    DIR *d;
    struct dirent *e;
    int found = 0;
    sal_snprintf(base, sizeof base, "/usr/lib/gcc/%s-w64-mingw32", arch);
    d = opendir(base);
    if (!d) return 0;
    while ((e = readdir(d)) != NULL) {
        char p[600];
        FILE *f;
        if (e->d_name[0] == '.') continue;
        sal_snprintf(p, sizeof p, "%s/%s/crtbegin.o", base, e->d_name);
        f = fopen(p, "rb");
        if (f) {
            fclose(f);
            sal_snprintf(out, n, "%s/%s", base, e->d_name);
            found = 1;
            if (strstr(e->d_name, "win32")) break;
        }
    }
    closedir(d);
    return found;
#    endif
}

static int find_compiler_rt(const char *rtarch, char *out, size_t n)
{
#    if defined(_WIN32)
    (void)rtarch;
    (void)out;
    (void)n;
    return 0;
#    else
    static const char *bases[] = {"/usr/lib/llvm-22/lib/clang", "/usr/lib/llvm/lib/clang",
                                  "/usr/lib/clang"};
    size_t bi = 0;
    for (; bi < sizeof bases / sizeof bases[0]; bi++) {
        DIR *d = opendir(bases[bi]);
        struct dirent *e;
        if (!d) continue;
        while ((e = readdir(d)) != NULL) {
            char p[700];
            FILE *f;
            if (e->d_name[0] == '.') continue;
            sal_snprintf(p, sizeof p, "%s/%s/lib/linux/libclang_rt.builtins-%s.a",
                         bases[bi], e->d_name, rtarch);
            f = fopen(p, "rb");
            if (f) {
                fclose(f);
                sal_snprintf(out, n, "%s", p);
                closedir(d);
                return 1;
            }
        }
        closedir(d);
    }
    return 0;
#    endif
}

/*
 * Materialize the embedded musl sysroot for `arch` (x86_64/aarch64/i386/arm), if
 * one was compiled into this salam. Each arch is a separate embedded tar with its
 * own symbol; only the ones staged at build time are present. Returns 1 and fills
 * `sr` on success, 0 otherwise. `arch` matches native_link_elf's naming, while the
 * cache name uses the canonical musl toolchain triple.
 */
static int salam_try_embed_musl(logger_t *log, const char *arch, char *sr, size_t srn)
{
    (void)log;
    (void)arch;
    (void)sr;
    (void)srn;
#    ifdef SALAM_HAVE_EMBED_MUSL
    if (strcmp(arch, "x86_64") == 0 &&
        salam_materialize_sysroot("x86_64-linux-musl", salam_embed_musl,
                                  (size_t)(salam_embed_musl_end - salam_embed_musl), sr,
                                  srn)) {
        LOG_I(log, PH_DRIVER, "using embedded musl sysroot: %s", sr);
        return 1;
    }
#    endif
#    ifdef SALAM_HAVE_EMBED_MUSL_AARCH64
    if (strcmp(arch, "aarch64") == 0 &&
        salam_materialize_sysroot(
            "aarch64-linux-musl", salam_embed_musl_aarch64,
            (size_t)(salam_embed_musl_aarch64_end - salam_embed_musl_aarch64), sr, srn)) {
        LOG_I(log, PH_DRIVER, "using embedded musl sysroot: %s", sr);
        return 1;
    }
#    endif
#    ifdef SALAM_HAVE_EMBED_MUSL_I686
    if (strcmp(arch, "i386") == 0 &&
        salam_materialize_sysroot(
            "i686-linux-musl", salam_embed_musl_i686,
            (size_t)(salam_embed_musl_i686_end - salam_embed_musl_i686), sr, srn)) {
        LOG_I(log, PH_DRIVER, "using embedded musl sysroot: %s", sr);
        return 1;
    }
#    endif
#    ifdef SALAM_HAVE_EMBED_MUSL_ARM
    if (strcmp(arch, "arm") == 0 &&
        salam_materialize_sysroot(
            "arm-linux-musleabihf", salam_embed_musl_arm,
            (size_t)(salam_embed_musl_arm_end - salam_embed_musl_arm), sr, srn)) {
        LOG_I(log, PH_DRIVER, "using embedded musl sysroot: %s", sr);
        return 1;
    }
#    endif
    return 0;
}

static int native_link_elf(logger_t *log, const char *obj, const char *out,
                           const codegen_llvm_options_t *opts)
{
    const char *t = opts->target_triple;
    const char *arch, *emul, *rtarch;
    char sr[1024], crt1[1200], crti[1200], crtn[1200], Lsr[1100], rt[1200];
    char userlibs[16][160];
    const char *argv[64];
    int n = 0, i, rc, have_rt, have_gcc = 0;
    FILE *f;

    if (strstr(t, "aarch64") || strstr(t, "arm64")) {
        arch = "aarch64";
        emul = "aarch64linux";
        rtarch = "aarch64";
    } else if (strstr(t, "i686") || strstr(t, "i386") || strstr(t, "i586")) {
        arch = "i386";
        emul = "elf_i386";
        rtarch = "i386";
    } else if (strstr(t, "arm")) { /* 32-bit arm (armhf / armv7, eabi) */
        arch = "arm";
        emul = "armelf_linux_eabi";
        rtarch = "arm";
    } else {
        arch = "x86_64";
        emul = "elf_x86_64";
        rtarch = "x86_64";
    }

    if (opts->sysroot && opts->sysroot[0]) {
        sal_snprintf(sr, sizeof sr, "%s", opts->sysroot);
    } else {
        int got = salam_try_embed_musl(log, arch, sr, sizeof sr);
        if (!got) {
            const char *env = getenv("SALAM_SYSROOTS");
            if (env && env[0]) {
                char probe[1200];
                sal_snprintf(sr, sizeof sr, "%s/%s-linux-musl", env, arch);
                sal_snprintf(probe, sizeof probe, "%s/crt1.o", sr);
                f = fopen(probe, "rb");
                if (f) {
                    fclose(f);
                    got = 1;
                }
            }
        }
        if (!got) sal_snprintf(sr, sizeof sr, "/usr/lib/%s-linux-musl", arch);
    }

    sal_snprintf(crt1, sizeof crt1, "%s/crt1.o", sr);
    f = fopen(crt1, "rb");
    if (!f) {
        LOG_E(log, PH_DRIVER,
              i18n_tr("no musl sysroot for '%s' (looked in %s): install musl-dev or "
                      "bundle a sysroot ($SALAM_SYSROOTS)"),
              t, sr);
        return 1;
    }
    fclose(f);
    sal_snprintf(crti, sizeof crti, "%s/crti.o", sr);
    sal_snprintf(crtn, sizeof crtn, "%s/crtn.o", sr);
    sal_snprintf(rt, sizeof rt, "%s/libclang_rt.builtins-%s.a", sr, rtarch);
    f = fopen(rt, "rb");
    if (f) {
        fclose(f);
        have_rt = 1;
    } else {
        have_rt = find_compiler_rt(rtarch, rt, sizeof rt);
    }
    {
        char gcc[1200];
        sal_snprintf(gcc, sizeof gcc, "%s/libgcc.a", sr);
        f = fopen(gcc, "rb");
        if (f) {
            fclose(f);
            have_gcc = 1;
        }
    }

    argv[n++] = "ld.lld";
    argv[n++] = "-m";
    argv[n++] = emul;
    argv[n++] = "-static";
    if (!opts->debug_info) argv[n++] = "-s"; /* strip symbols unless -g */
    argv[n++] = "-o";
    argv[n++] = out;
    argv[n++] = crt1;
    argv[n++] = crti;
    sal_snprintf(Lsr, sizeof Lsr, "-L%s", sr);
    argv[n++] = Lsr;
    argv[n++] = obj;
    for (i = 0; i < opts->nlink && i < 16 && n < 50; i++) {
        const char *s = opts->link_libs[i];
        if (!s) continue;
        if (s[0] == '-' || strpbrk(s, "/\\.") != NULL) {
            argv[n++] = s;
        } else {
            sal_snprintf(userlibs[i], sizeof userlibs[i], "-l%s", s);
            argv[n++] = userlibs[i];
        }
    }
    if (opts->nlink > 16)
        LOG_W(log, PH_DRIVER,
              i18n_tr("too many link libraries (%d); only the first 16 were linked"),
              opts->nlink);
    argv[n++] = "--start-group";
    argv[n++] = "-lc";
    if (have_rt) argv[n++] = rt;
    if (have_gcc) argv[n++] = "-lgcc";
    argv[n++] = "--end-group";
    argv[n++] = crtn;

    LOG_I(log, PH_DRIVER,
          "in-process lld: linking %s -> %s (%s, static musl, sysroot %s)", obj, out, t,
          sr);
    rc = salam_lld_link(SALAM_LLD_ELF, n, argv);
    if (rc != 0) LOG_E(log, PH_DRIVER, i18n_tr("in-process lld link failed (%d)"), rc);
    return rc == 0 ? 0 : 1;
}

static int native_link_mingw(logger_t *log, const char *obj, const char *out,
                             const codegen_llvm_options_t *opts)
{
    const char *t = opts->target_triple;
    const char *arch, *emul;
    char sr[1024], crt2[1200], crtbegin[1200], crtend[1200];
    char Lgcc[1100], Lsr[1100], Lsrmingw[1100], gccdir[1024];
    char userlibs[16][160];
    const char *argv[96];
    int n = 0, have_gcc, i, rc;
    FILE *f;
    static const char *group[] = {"-lmingw32", "-lgcc",     "-lgcc_eh",   "-lmoldname",
                                  "-lmingwex", "-lmsvcrt",  "-ladvapi32", "-lshell32",
                                  "-luser32",  "-lkernel32"};

    if (!lld_can_link(t)) return -1;
    arch = mingw_arch(t);
    emul = mingw_emul(t);

    if (opts->sysroot && opts->sysroot[0]) {
        sal_snprintf(sr, sizeof sr, "%s", opts->sysroot);
    } else {
        int got = 0;
#    ifdef SALAM_HAVE_EMBED_MINGW
        /* Prefer the sysroot baked into the binary: fully self-contained. */
        if (strcmp(arch, "x86_64") == 0 &&
            salam_materialize_sysroot("x86_64-w64-mingw32", salam_embed_mingw,
                                      (size_t)(salam_embed_mingw_end - salam_embed_mingw),
                                      sr, sizeof sr)) {
            got = 1;
            LOG_I(log, PH_DRIVER, "using embedded mingw sysroot: %s", sr);
        }
#    endif
        if (!got && !salam_find_sysroot(t, sr, sizeof sr))
            sal_snprintf(sr, sizeof sr, "/usr/%s-w64-mingw32", arch);
    }

    sal_snprintf(crt2, sizeof crt2, "%s/lib/crt2.o", sr);
    f = fopen(crt2, "rb");
    if (!f) {
        LOG_E(log, PH_DRIVER,
              i18n_tr("no mingw sysroot for '%s' (looked in %s): install mingw-w64 or "
                      "bundle a sysroot ($SALAM_SYSROOTS)"),
              t, sr);
        return 1;
    }
    fclose(f);
    sal_snprintf(gccdir, sizeof gccdir, "%s/lib", sr);
    {
        char probe[1200];
        sal_snprintf(probe, sizeof probe, "%s/libgcc.a", gccdir);
        f = fopen(probe, "rb");
        if (f) {
            fclose(f);
            have_gcc = 1;
        } else {
            have_gcc = find_mingw_gccdir(arch, gccdir, sizeof gccdir);
        }
    }

    argv[n++] = "ld.lld";
    argv[n++] = "-m";
    argv[n++] = emul;
    argv[n++] = "-Bdynamic";
    if (!opts->debug_info) argv[n++] = "-s"; /* strip symbols unless -g */
    argv[n++] = "-o";
    argv[n++] = out;
    argv[n++] = crt2;
    if (have_gcc) {
        sal_snprintf(crtbegin, sizeof crtbegin, "%s/crtbegin.o", gccdir);
        argv[n++] = crtbegin;
        sal_snprintf(Lgcc, sizeof Lgcc, "-L%s", gccdir);
        argv[n++] = Lgcc;
    }
    sal_snprintf(Lsr, sizeof Lsr, "-L%s/lib", sr);
    argv[n++] = Lsr;
    sal_snprintf(Lsrmingw, sizeof Lsrmingw, "-L%s/mingw/lib", sr);
    argv[n++] = Lsrmingw;
    argv[n++] = obj;
    for (i = 0; i < opts->nlink && i < 16 && n < 70; i++) {
        const char *s = opts->link_libs[i];
        if (!s) continue;
        if (s[0] == '-' || strpbrk(s, "/\\.") != NULL) {
            argv[n++] = s;
        } else {
            sal_snprintf(userlibs[i], sizeof userlibs[i], "-l%s", s);
            argv[n++] = userlibs[i];
        }
    }
    if (opts->nlink > 16)
        LOG_W(log, PH_DRIVER,
              i18n_tr("too many link libraries (%d); only the first 16 were linked"),
              opts->nlink);
    /* default mingw libs, wrapped in a group to resolve their circular deps */
    argv[n++] = "--start-group";
    for (i = 0; i < (int)(sizeof group / sizeof group[0]) && n < 88; i++)
        argv[n++] = group[i];
    argv[n++] = "--end-group";
    if (have_gcc) {
        sal_snprintf(crtend, sizeof crtend, "%s/crtend.o", gccdir);
        argv[n++] = crtend;
    }

    LOG_I(log, PH_DRIVER, "in-process lld: linking %s -> %s (%s)", obj, out, t);
    rc = salam_lld_link(SALAM_LLD_MINGW, n, argv);
    if (rc != 0) LOG_E(log, PH_DRIVER, i18n_tr("in-process lld link failed (%d)"), rc);
    return rc == 0 ? 0 : 1;
}

static int native_link_lld(logger_t *log, const char *obj, const char *out,
                           const codegen_llvm_options_t *opts)
{
    const char *t = opts->target_triple;
    if (!t) return -1;
    if ((strstr(t, "windows") || strstr(t, "mingw") || strstr(t, "win32")) &&
        (strstr(t, "gnu") || strstr(t, "mingw")))
        return native_link_mingw(log, obj, out, opts);
    if (strstr(t, "linux") && strstr(t, "musl"))
        return native_link_elf(log, obj, out, opts);
    return -1;
}
#  endif

static int orc_fail(logger_t *log, const char *what, LLVMErrorRef e)
{
    char *msg = LLVMGetErrorMessage(e);
    LOG_E(log, PH_DRIVER, "%s: %s", what, msg ? msg : "(unknown)");
    if (msg) LLVMDisposeErrorMessage(msg);
    return 1;
}

static void define_runtime_symbols(LLVMOrcLLJITRef jit, LLVMOrcJITDylibRef jd)
{
#  define RT_FLAGS                                                                       \
      {LLVMJITSymbolGenericFlagsExported | LLVMJITSymbolGenericFlagsCallable, 0}
#  define RT_SYM(fn)                                                                     \
      {                                                                                  \
          LLVMOrcLLJITMangleAndIntern(jit, #fn),                                         \
          {                                                                              \
              (LLVMOrcExecutorAddress)(size_t) & fn, RT_FLAGS                            \
          }                                                                              \
      }
    LLVMOrcCSymbolMapPair syms[] = {RT_SYM(printf), RT_SYM(snprintf), RT_SYM(strlen),
                                    RT_SYM(strcmp), RT_SYM(malloc),   RT_SYM(realloc),
                                    RT_SYM(free),   RT_SYM(memcpy),   RT_SYM(memmove),
                                    RT_SYM(abort),  RT_SYM(exit),     RT_SYM(strtol),
                                    RT_SYM(strtod), RT_SYM(strstr)};
    LLVMOrcJITDylibDefine(jd, LLVMOrcAbsoluteSymbols(syms, sizeof syms / sizeof syms[0]));
#  if defined(_WIN32)
    {
        extern void __main(void);
        LLVMOrcCSymbolMapPair w[] = {
            {LLVMOrcLLJITMangleAndIntern(jit, "__main"),
             {(LLVMOrcExecutorAddress)(size_t)&__main, RT_FLAGS}}};
        LLVMOrcJITDylibDefine(jd, LLVMOrcAbsoluteSymbols(w, 1));
    }
#  endif
#  undef RT_SYM
#  undef RT_FLAGS
}

static int jit_run_file(logger_t *log, const char *ll_path)
{
    LLVMInitializeNativeTarget();
    LLVMInitializeNativeAsmPrinter();

#  if defined(SALAM_LLVM_VERSION_MAJOR) && SALAM_LLVM_VERSION_MAJOR >= 21
    LLVMContextRef ctx = LLVMContextCreate();
    LLVMOrcThreadSafeContextRef tsctx =
        LLVMOrcCreateNewThreadSafeContextFromLLVMContext(ctx);
#  else
    LLVMOrcThreadSafeContextRef tsctx = LLVMOrcCreateNewThreadSafeContext();
    LLVMContextRef ctx = LLVMOrcThreadSafeContextGetContext(tsctx);
#  endif

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
    if (e) {
        LLVMOrcDisposeThreadSafeModule(tsm);
        return orc_fail(log, "JIT init failed", e);
    }

    LLVMOrcJITDylibRef jd = LLVMOrcLLJITGetMainJITDylib(jit);

    LLVMOrcDefinitionGeneratorRef gen = NULL;
    e = LLVMOrcCreateDynamicLibrarySearchGeneratorForProcess(
        &gen, LLVMOrcLLJITGetGlobalPrefix(jit), NULL, NULL);
    if (e) {
        LLVMOrcDisposeLLJIT(jit);
        LLVMOrcDisposeThreadSafeModule(tsm);
        return orc_fail(log, "JIT symbol generator failed", e);
    }
    LLVMOrcJITDylibAddGenerator(jd, gen);
    define_runtime_symbols(jit, jd);

    e = LLVMOrcLLJITAddLLVMIRModule(jit, jd, tsm);
    if (e) {
        LLVMOrcDisposeLLJIT(jit);
        return orc_fail(log, "JIT add module failed", e);
    }

    LLVMOrcExecutorAddress addr = 0;
    e = LLVMOrcLLJITLookup(jit, &addr, "main");
    if (e) {
        LLVMOrcDisposeLLJIT(jit);
        return orc_fail(log, i18n_tr("no 'main' function to run"), e);
    }
    if (!addr) {
        LLVMOrcDisposeLLJIT(jit);
        LOG_E(log, PH_DRIVER, i18n_tr("no 'main' function to run"));
        return 1;
    }

    int (*main_fn)(void) = (int (*)(void))(size_t)addr;
    int rc = main_fn();

    LLVMOrcDisposeLLJIT(jit);
    return rc;
}

int salam_llvm_native(logger_t *log, const char *ll_path,
                      const codegen_llvm_options_t *opts)
{
    init_targets_once();

    if (opts->output_mode == LLVM_OUT_JIT) return jit_run_file(log, ll_path);

    if (opts->output_mode == LLVM_OUT_EXEC && opts->target_triple &&
        opts->target_triple[0]) {
        char *ht = LLVMGetDefaultTargetTriple();
        int cross = !ht || strcmp(ht, opts->target_triple) != 0;
        if (ht) LLVMDisposeMessage(ht);
        if (cross && !lld_can_link(opts->target_triple)) return -1;
    }

    LLVMContextRef ctx = NULL;
    LLVMModuleRef mod = NULL;
    if (parse_ir(log, ll_path, &ctx, &mod) != 0) return 2;

    char *host_triple = LLVMGetDefaultTargetTriple();
    const char *triple = (opts->target_triple && opts->target_triple[0])
                             ? opts->target_triple
                             : host_triple;
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

    /*
     * IR output at -O0 is the raw front-end IR, emitted as-is. At -O1+ we run
     * the middle-end pipeline and rewrite the .ll so `--emit-llvm -O3` shows
     * the IR that actually runs (matching `clang -O3 -emit-llvm`), not the
     * unoptimized front-end dump.
     */
    if (opts->output_mode == LLVM_OUT_IR && opts->opt_level == LLVM_OPT_O0) {
        goto cleanup;
    }

    {
        LLVMTargetRef target = NULL;
        char *terr = NULL;
        if (LLVMGetTargetFromTriple(triple, &target, &terr)) {
            LOG_E(log, PH_DRIVER, "LLVM target '%s': %s", triple,
                  terr ? terr : "(unavailable)");
            if (terr) LLVMDisposeMessage(terr);
            rc = 2;
            goto cleanup;
        }
        char *host_cpu = NULL;
        char *host_features = NULL;
        int is_host_target =
            !opts->target_triple || !opts->target_triple[0] ||
            (host_triple && strcmp(opts->target_triple, host_triple) == 0);
        if (opts->native_cpu && is_host_target) {
            host_cpu = LLVMGetHostCPUName();
            host_features = LLVMGetHostCPUFeatures();
            LOG_I(log, PH_DRIVER, "tuning for host CPU: %s",
                  host_cpu ? host_cpu : "(unknown)");
        }
        LLVMTargetMachineRef tm = LLVMCreateTargetMachine(
            target, triple, host_cpu ? host_cpu : "", host_features ? host_features : "",
            map_cg_level(opts->opt_level), LLVMRelocPIC, LLVMCodeModelDefault);
        if (host_cpu) LLVMDisposeMessage(host_cpu);
        if (host_features) LLVMDisposeMessage(host_features);
        if (!tm) {
            LOG_E(log, PH_DRIVER, "could not create LLVM target machine for %s", triple);
            rc = 2;
            goto cleanup;
        }

        {
            LLVMTargetDataRef td = LLVMCreateTargetDataLayout(tm);
            LLVMSetModuleDataLayout(mod, td);
            LLVMDisposeTargetData(td);
        }

        run_opt(log, mod, tm, opts->opt_level);

        if (opts->output_mode == LLVM_OUT_IR) {
            char *emit_err = NULL;
            if (LLVMPrintModuleToFile(mod, ll_path, &emit_err)) {
                LOG_E(log, PH_DRIVER, "LLVM IR write failed: %s",
                      emit_err ? emit_err : "(unknown)");
                if (emit_err) LLVMDisposeMessage(emit_err);
                rc = 2;
            }
            LLVMDisposeTargetMachine(tm);
            goto cleanup;
        }

        if (opts->output_mode == LLVM_OUT_BITCODE) {
            if (LLVMWriteBitcodeToFile(mod, opts->output_file) != 0) {
                LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), opts->output_file);
                rc = 2;
            }
        } else if (opts->output_mode == LLVM_OUT_ASM ||
                   opts->output_mode == LLVM_OUT_OBJ) {
            LLVMCodeGenFileType ft =
                (opts->output_mode == LLVM_OUT_ASM) ? LLVMAssemblyFile : LLVMObjectFile;
            char *eerr = NULL;
            if (LLVMTargetMachineEmitToFile(tm, mod, opts->output_file, ft, &eerr)) {
                LOG_E(log, PH_DRIVER, "LLVM emit failed: %s", eerr ? eerr : "(unknown)");
                if (eerr) LLVMDisposeMessage(eerr);
                rc = 2;
            }
        } else if (opts->output_mode == LLVM_OUT_EXEC) {
            char objpath[1024];
            sal_snprintf(objpath, sizeof objpath, "%s.o", opts->output_file);
            char *eerr = NULL;
            if (LLVMTargetMachineEmitToFile(tm, mod, objpath, LLVMObjectFile, &eerr)) {
                LOG_E(log, PH_DRIVER, "LLVM emit failed: %s", eerr ? eerr : "(unknown)");
                if (eerr) LLVMDisposeMessage(eerr);
                rc = 2;
            } else {
                rc = -1;
#  ifdef SALAM_HAVE_LLD
                if (opts->target_triple && opts->target_triple[0])
                    rc = native_link_lld(log, objpath, opts->output_file, opts);
#  endif
                if (rc < 0) rc = link_executable(log, objpath, opts->output_file, opts);
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

#endif
