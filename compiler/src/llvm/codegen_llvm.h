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

#ifndef SALAM_LLVM_CODEGEN_LLVM_H
#define SALAM_LLVM_CODEGEN_LLVM_H

#include "core/prelude.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "ast/ast.h"
#include "semantic/sema.h"

typedef enum {
    LLVM_OPT_O0 = 0,   /* no optimization (fast build, best debugging) */
    LLVM_OPT_O1,       /* basic scalar opts                           */
    LLVM_OPT_O2,       /* default speed pipeline                      */
    LLVM_OPT_O3,       /* aggressive (vectorize, slp, ...)            */
    LLVM_OPT_OS,       /* optimize for size                          */
    LLVM_OPT_OZ        /* optimize for size, aggressively            */
} llvm_opt_level_t;

typedef enum {
    LLVM_OUT_IR = 0,   /* textual LLVM IR (.ll)                       */
    LLVM_OUT_ASM,      /* target assembly (.s) via llc               */
    LLVM_OUT_OBJ,      /* object file (.o) via llc                    */
    LLVM_OUT_BITCODE,  /* LLVM bitcode (.bc) via llvm-as/opt (LTO)    */
    LLVM_OUT_EXEC,     /* native executable via clang (links libc/m)  */
    LLVM_OUT_JIT       /* run immediately via lli; no artifact kept   */
} llvm_output_mode_t;

typedef struct {
    llvm_opt_level_t   opt_level;      /* -O0..-Oz                              */
    bool               debug_info;     /* -g: emit DWARF metadata into the IR   */
    bool               verify_module;  /* --verify-ir: run the LLVM verifier    */
    llvm_output_mode_t output_mode;    /* --emit-* / --exec / --jit             */
    const char        *output_file;    /* -o FILE (NULL = derive from module)   */
    const char        *target_triple;  /* --target=TRIPLE (NULL = host)         */
} codegen_llvm_options_t;

void codegen_llvm_options_default(codegen_llvm_options_t *o);

typedef struct {
    const char *module;
    const char *ll_src;
    bool        ok;
    const char *error;
} llvm_output_t;

llvm_output_t *codegen_llvm_run(arena_t *a, logger_t *log, ast_node_t *program,
                                sema_result_t *sem, const char *module,
                                const char *entry);

llvm_output_t *codegen_llvm_run_opts(arena_t *a, logger_t *log, ast_node_t *program,
                                     sema_result_t *sem, const char *module,
                                     const char *entry,
                                     const codegen_llvm_options_t *opts,
                                     const char *src_path);

const char *codegen_llvm_error(const llvm_output_t *out);

#endif /* SALAM_LLVM_CODEGEN_LLVM_H */
