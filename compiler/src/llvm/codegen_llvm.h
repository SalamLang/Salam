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
    LLVM_OPT_O0 = 0,
    LLVM_OPT_O1,
    LLVM_OPT_O2,
    LLVM_OPT_O3,
    LLVM_OPT_OS,
    LLVM_OPT_OZ
} llvm_opt_level_t;

typedef enum {
    LLVM_OUT_IR = 0,
    LLVM_OUT_ASM,
    LLVM_OUT_OBJ,
    LLVM_OUT_BITCODE,
    LLVM_OUT_EXEC,
    LLVM_OUT_JIT
} llvm_output_mode_t;

typedef struct {
    llvm_opt_level_t opt_level;
    bool debug_info;
    bool verify_module;
    llvm_output_mode_t output_mode;
    const char *output_file;
    const char *target_triple;
    /* Libraries from `link "..."` directives, honored when linking an
     * executable. link_kinds[i] mirrors the AST_LINK node kind (e.g. "static",
     * "framework") or NULL for a plain dynamic library. */
    const char *const *link_libs;
    const char *const *link_kinds;
    int nlink;
    /* Optional cross-compilation sysroot (mingw-w64 etc.) passed to clang. */
    const char *sysroot;
} codegen_llvm_options_t;

void codegen_llvm_options_default(codegen_llvm_options_t *o);

typedef struct {
    const char *module;
    const char *ll_src;
    bool ok;
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
