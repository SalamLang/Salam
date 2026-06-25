#ifndef SALAM_DRIVER_LLVM_TOOLCHAIN_H
#define SALAM_DRIVER_LLVM_TOOLCHAIN_H

#include "core/prelude.h"
#include "logger/logger.h"
#include "llvm/codegen_llvm.h"

int salam_llvm_toolchain(logger_t *log, const char *ll_path,
                         const codegen_llvm_options_t *opts);

#endif /* SALAM_DRIVER_LLVM_TOOLCHAIN_H */
