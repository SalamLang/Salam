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
 * MSYS2's mingw-w64-x86_64-lld and mingw-w64-x86_64-llvm packages ship
 * liblld{ELF,COFF,MinGW,MachO,Wasm,Common}.a and libLLVMWindowsManifest.a
 * pre-built as if they will always be consumed alongside the shared
 * libLLVM-*.dll / libxml2 DLL: their object code reaches the demangler and
 * libxml2 through __imp_-prefixed indirect pointers (the standard PE
 * dllimport calling convention), even though the real implementations are
 * also available as plain static archives in this very link
 * (libLLVMDemangle.a, libxml2.a). Linking against libLLVM-*.dll.a /
 * libxml2.dll.a to satisfy those __imp_ symbols would pull in a runtime
 * dependency on libLLVM-*.dll / libxml2-*.dll, defeating the point of a
 * self-contained salam.exe (see the "Verify salam.exe is self-contained"
 * CI step). Since a dllimport-style reference is just an extra pointer
 * indirection - call/load through __imp_X instead of calling/using X
 * directly - defining each __imp_X here as a plain data pointer aliasing
 * the real, statically-linked X satisfies the linker without needing any
 * DLL at runtime.
 */

.section .data
.align 8

#define IMPORT_ALIAS(name)      \
    .globl __imp_##name;        \
    __imp_##name:                \
    .quad name

/* llvm::demangle() / llvm::ItaniumPartialDemangler, from libLLVMDemangle.a -
 * needed by liblldELF.a, liblldCOFF.a, liblldMachO.a and liblldWasm.a. */
IMPORT_ALIAS(_ZN4llvm8demangleB5cxx11ESt17basic_string_viewIcSt11char_traitsIcEE)
IMPORT_ALIAS(_ZN4llvm23ItaniumPartialDemanglerC1Ev)
IMPORT_ALIAS(_ZN4llvm23ItaniumPartialDemanglerD1Ev)
IMPORT_ALIAS(_ZN4llvm23ItaniumPartialDemangler15partialDemangleEPKc)
IMPORT_ALIAS(_ZNK4llvm23ItaniumPartialDemangler15getFunctionNameEPcPy)

/* libxml2, from libxml2.a - needed by libLLVMWindowsManifest.a (pulled in
 * transitively by liblldCOFF.a's manifest-merge support). */
IMPORT_ALIAS(xmlAddChild)
IMPORT_ALIAS(xmlCopyNamespace)
IMPORT_ALIAS(xmlDocDumpFormatMemoryEnc)
IMPORT_ALIAS(xmlDocGetRootElement)
IMPORT_ALIAS(xmlDocSetRootElement)
IMPORT_ALIAS(xmlFree)
IMPORT_ALIAS(xmlFreeDoc)
IMPORT_ALIAS(xmlFreeNode)
IMPORT_ALIAS(xmlFreeNs)
IMPORT_ALIAS(xmlNewDoc)
IMPORT_ALIAS(xmlNewNs)
IMPORT_ALIAS(xmlNewProp)
IMPORT_ALIAS(xmlReadMemory)
IMPORT_ALIAS(xmlSetGenericErrorFunc)
IMPORT_ALIAS(xmlStrdup)
IMPORT_ALIAS(xmlUnlinkNode)
