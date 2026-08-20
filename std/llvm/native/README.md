# std/llvm/native

The only C and C++ in the Salam toolchain that is not going away.

Everything else the compiler is made of has a self-hosted counterpart under
`compiler/`. These six files do not, and cannot, because of how LLVM and LLD
publish their interfaces:

- **`orc_call.c` / `orc_call.h`** — `LLVMInitializeAllTargets`,
  `...AllTargetInfos`, `...AllTargetMCs`, `...AllAsmPrinters`,
  `...AllAsmParsers` are **preprocessor macros** in LLVM's C API, not
  functions, so nothing outside C can call them. This wraps each one in a real
  function. It also carries `salam_orc_call_main`, which calls a JIT'd address
  through a function pointer.
- **`lld_link.cc` / `lld_link.h`** — LLD has **no C API at all**; its entry
  points are C++ (`lld::coff::link` and friends). This is the C++ wrapper that
  gives them C linkage.
- **`lld_stub.c`** — the same two symbols as `lld_link.cc`, for builds without
  in-process LLD, so `std/llvm` still resolves.
- **`win_lld_demangle_shim.S`** — a Windows link-time shim; see the comment in
  the file.

That is eight exported symbols in total:

    salam_orc_call_main                 salam_llvm_init_all_targets
    salam_llvm_init_all_target_infos    salam_llvm_init_all_target_mcs
    salam_llvm_init_all_asm_printers    salam_llvm_init_all_asm_parsers
    salam_lld_available                 salam_lld_link

Every other native symbol `std/llvm` binds is an `LLVM*` entry point that comes
from libLLVM itself, so it needs no wrapper of ours.

They live here, next to the `std/llvm` bindings that declare them, rather than
in `c/`, because `c/` is the old C implementation of the compiler and is being
removed. These files outlive it.

They are compiled into `libsalam_llvm.a` together with the LLVM and LLD static
archives; `std/llvm/llvm.salam` then links it with `link static "salam_llvm"`.
