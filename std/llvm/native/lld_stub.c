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
 * The two entry points lld_link.cc exports, for builds that have no
 * in-process LLD.
 *
 * Nothing in c/ calls either without an `#ifdef SALAM_HAVE_LLD`, so the C
 * compiler does not need them - the self-hosted one does. std/llvm/lld.salam
 * declares both `extern:`, and importing a Salam package links every extern
 * its pub functions name, so libsalam_llvm.a has to carry the symbols
 * whether or not LLD went into it.
 *
 * That is the normal configuration on macOS: Homebrew builds its lld formula
 * with BUILD_SHARED_LIBS=ON, so there are no liblld*.a to merge at all. LLVM
 * codegen still runs in-process; only the final link shells out, which is
 * what a macOS host does with LLD linked in anyway (native_link_lld/
 * CanLinkTriple only ever claim the mingw-gnu and linux-musl triples).
 *
 * Why its own translation unit rather than a corner of llvm_native.c: an
 * archive member is pulled in by symbol, and it drags its own undefined
 * references in with it. llvm_native.c calls the compiler's core (sb_*,
 * sal_snprintf, logger_log, i18n_tr, salam_find_bundled_tool), none of which
 * exists in a self-hosted link - nothing in std/llvm references
 * salam_llvm_native, so that object is never pulled and its dependencies
 * never matter. Putting these two functions there instead would give
 * std/llvm a reason to pull it, and the stage 2 link fails on nine
 * undefined core symbols. Keep this file dependency-free.
 */

#include "lld_link.h"

#ifdef SALAM_HAVE_LLD

/*
 * lld_link.cc owns both symbols in this configuration. The typedef only
 * exists so the translation unit is not empty, which ISO C does not allow.
 */
typedef int salam_lld_stub_unused_t;

#else

int salam_lld_available(void)
{
    return 0;
}

int salam_lld_link(int flavor, int argc, const char *const *argv)
{
    (void)flavor;
    (void)argc;
    (void)argv;
    return 1;
}

#endif /* SALAM_HAVE_LLD */
