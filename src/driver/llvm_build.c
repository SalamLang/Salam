#include "core/prelude.h"
#include "driver/llvm_build.h"
#include "core/arena.h"
#include "logger/logger.h"
#include "langpack/langpack.h"
#include "source/source.h"
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/sema.h"
#include "preproc/preproc.h"
#include "llvm/codegen_llvm.h"
#include "driver/llvm_toolchain.h"
#include "i18n/i18n.h"

static const char *module_of(arena_t *a, const char *path)
{
    const char *slash = strrchr(path, '/');
    const char *bslash = strrchr(path, '\\');
    const char *base = path;
    if (slash && slash + 1 > base) base = slash + 1;
    if (bslash && bslash + 1 > base) base = bslash + 1;
    const char *dot = strrchr(base, '.');
    size_t len = dot ? (size_t)(dot - base) : strlen(base);
    return arena_strndup(a, base, len);
}

static const char *default_output(arena_t *a, const char *module, llvm_output_mode_t m)
{
    const char *ext = ".ll";
    switch (m) {
        case LLVM_OUT_ASM:     ext = ".s";   break;
        case LLVM_OUT_OBJ:     ext = ".o";   break;
        case LLVM_OUT_BITCODE: ext = ".bc";  break;
        case LLVM_OUT_EXEC:    ext = "";     break;   
        default:               ext = ".ll";  break;
    }
    char *o = (char *)arena_alloc(a, strlen(module) + strlen(ext) + 1);
    sprintf(o, "%s%s", module, ext);
    return o;
}

int driver_llvm(options_t *opt)
{
    logger_t *log   = logger_new(stderr, opt->log_level, opt->color == 1);
    arena_t  *arena = arena_new(1 << 20);
    int rc = 0;
    langpack_t *pack = langpack_load(opt->lang);
    if (!pack) { LOG_E(log, PH_DRIVER, i18n_tr("unknown language pack '%s'"), opt->lang);
                 logger_free(log); arena_free(arena); return 2; }
    const char *entry = langpack_entry(pack);
    salam_set_stdlib_root(opt->stdlib_path);
    if (!opt->input) {
        LOG_E(log, PH_DRIVER, i18n_tr("salamc: no input file"));
        logger_free(log); arena_free(arena); return 2;
    }
    source_file_t *src = source_load(arena, opt->input);
    if (!src) { LOG_E(log, PH_DRIVER, i18n_tr("cannot read source file '%s'"), opt->input);
                logger_free(log); arena_free(arena); return 2; }
    src = preproc_source(arena, log, src, opt->defines, opt->ndefines);
    logger_set_diag_source(log, src->text, src->len, opt->diag_style, opt->diag_format);
    const char *module = module_of(arena, opt->input);
    token_stream_t *toks = NULL;
    bool lok = lexer_run(arena, log, pack, src, &toks);
    ast_node_t *program = NULL;
    bool pok = parser_run(arena, log, toks, &program);
    sema_result_t *sr = sema_run(arena, log, program, src->path);
    if (!lok || !pok || !sr->ok) {
        LOG_E(log, PH_DRIVER, i18n_tr("build aborted: errors in source"));
        logger_free(log); arena_free(arena); return 1;
    }
    
    codegen_llvm_options_t o;
    codegen_llvm_options_default(&o);
    o.opt_level     = (llvm_opt_level_t)opt->llvm_opt_level;
    o.output_mode   = (llvm_output_mode_t)opt->llvm_emit;
    o.debug_info    = opt->debug_info;
    o.verify_module = opt->llvm_verify;
    o.target_triple = opt->llvm_target;
    llvm_output_t *out = codegen_llvm_run_opts(arena, log, program, sr, module,
                                               entry, &o, src->path);
    
    bool ir_mode = (o.output_mode == LLVM_OUT_IR);
    const char *llpath = ir_mode && opt->output ? opt->output
                       : (char *)arena_alloc(arena, strlen(module) + 4);
    if (!(ir_mode && opt->output)) sprintf((char *)llpath, "%s.ll", module);
    FILE *f = fopen(llpath, "wb");
    if (!f) { LOG_E(log, PH_DRIVER, i18n_tr("cannot write '%s'"), llpath);
              logger_free(log); arena_free(arena); return 2; }
    fputs(out->ll_src, f);
    fclose(f);
    if (!out->ok) {
        const char *e = codegen_llvm_error(out);
        LOG_W(log, PH_DRIVER,
              "%s; the program uses constructs the LLVM backend does not yet "
              "support, so %s may be incomplete", e ? e : "unsupported construct", llpath);
        logger_free(log); arena_free(arena); return 1;   
    }
    if (ir_mode) {
        if (o.verify_module) rc = salam_llvm_toolchain(log, llpath, &o);   
        if (rc == 0) {
            LOG_I(log, PH_DRIVER, "LLVM IR written to %s", llpath);
            LOG_I(log, PH_DRIVER, "  native: clang %s -lm -o %s | wasm: clang --target=wasm32-wasi ...",
                  llpath, module);
        }
    } else if (o.output_mode == LLVM_OUT_JIT) {
        rc = salam_llvm_toolchain(log, llpath, &o);
        if (!opt->keep_c) remove(llpath);
    } else {
        o.output_file = opt->output ? opt->output : default_output(arena, module, o.output_mode);
        rc = salam_llvm_toolchain(log, llpath, &o);
        if (!opt->keep_c) remove(llpath);
        if (rc == 0) {
            LOG_I(log, PH_DRIVER, "wrote %s", o.output_file);
            if (opt->exe_path[0] == '\0' && o.output_mode == LLVM_OUT_EXEC)
                snprintf(opt->exe_path, sizeof(opt->exe_path), "%s", o.output_file);
        }
    }
    logger_free(log);
    arena_free(arena);
    return rc;
}
