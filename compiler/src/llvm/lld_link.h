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

#ifndef SALAM_LLVM_LLD_LINK_H
#define SALAM_LLVM_LLD_LINK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SALAM_LLD_ELF = 0,   /* Linux/BSD ELF (GNU driver)          */
    SALAM_LLD_MINGW = 1, /* Windows via the GNU-style MinGW driver */
    SALAM_LLD_COFF = 2,  /* Windows via the MSVC link.exe driver   */
    SALAM_LLD_MACHO = 3, /* macOS Mach-O                        */
    SALAM_LLD_WASM = 4   /* WebAssembly                         */
} salam_lld_flavor_t;

int salam_lld_available(void);

int salam_lld_link(int flavor, int argc, const char *const *argv);

#ifdef __cplusplus
}
#endif

#endif /* SALAM_LLVM_LLD_LINK_H */
