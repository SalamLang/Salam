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
