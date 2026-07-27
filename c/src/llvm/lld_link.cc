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
 * In-process linker: calls the lld driver libraries directly so a
 * WITH_LLD=1 salam links executables without a separate ld.lld/clang process.
 */

#include "llvm/lld_link.h"

#include "lld/Common/Driver.h"
#include "llvm/Support/raw_ostream.h"

#include <vector>

LLD_HAS_DRIVER(elf)
LLD_HAS_DRIVER(mingw)
LLD_HAS_DRIVER(coff)
LLD_HAS_DRIVER(macho)
LLD_HAS_DRIVER(wasm)

extern "C" int salam_lld_available(void)
{
    return 1;
}

extern "C" int salam_lld_link(int flavor, int argc, const char *const *argv)
{
    std::vector<const char *> args(argv, argv + argc);
    llvm::ArrayRef<const char *> a(args.data(), args.size());
    bool ok = false;
    switch (flavor) {
    case SALAM_LLD_ELF:
        ok = lld::elf::link(a, llvm::outs(), llvm::errs(), false, false);
        break;
    case SALAM_LLD_MINGW:
        ok = lld::mingw::link(a, llvm::outs(), llvm::errs(), false, false);
        break;
    case SALAM_LLD_COFF:
        ok = lld::coff::link(a, llvm::outs(), llvm::errs(), false, false);
        break;
    case SALAM_LLD_MACHO:
        ok = lld::macho::link(a, llvm::outs(), llvm::errs(), false, false);
        break;
    case SALAM_LLD_WASM:
        ok = lld::wasm::link(a, llvm::outs(), llvm::errs(), false, false);
        break;
    default:
        return 2;
    }
    return ok ? 0 : 1;
}
