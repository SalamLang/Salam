#!/bin/sh
# Quick build of salamc.

set -e
. "$(dirname "$0")/lib.sh"
if [ -z "${CC:-}" ]; then
    if   command -v gcc   >/dev/null 2>&1; then CC=gcc
    elif command -v clang >/dev/null 2>&1; then CC=clang
    else CC=tcc
    fi
fi
case "$CC" in
    *tcc*) LDLIBS="" ;;
    *)     LDLIBS="-lm" ;;
esac
"$CC" -Wall -Isrc -o salamc \
    src/salamc.c \
    src/core/arena.c src/core/vec.c src/core/sb.c src/core/numstr.c src/core/sal_format.c \
    src/source/source.c src/logger/logger.c src/logger/logger_emit.c src/logger/logger_level.c \
    src/xml/xml.c src/xml/xml_emit.c \
    src/preproc/preproc.c src/preproc/preproc_cond.c \
    src/token/token.c src/token/token_xml.c \
    src/langpack/langpack.c src/langpack/langpack_data.c src/langpack/langpack_lookup.c \
    src/i18n/i18n.c src/i18n/i18n_locale.c src/i18n/i18n_gettext.c src/i18n/i18n_load.c \
    src/lexer/lexer.c src/lexer/lexer_cursor.c src/lexer/lexer_char.c \
    src/lexer/lexer_number.c src/lexer/lexer_string.c src/lexer/lexer_escape.c \
    src/lexer/lexer_trivia.c src/lexer/lexer_ident.c src/lexer/lexer_operator.c \
    src/lexer/lexer_layout.c \
    src/ast/ast.c src/ast/ast_xml.c \
    src/parser/parser.c src/parser/parser_cursor.c \
    src/parser/parser_ffi.c src/parser/parser_type.c \
    src/parser/parser_decl.c src/parser/parser_decl_var.c \
    src/parser/parser_stmt.c \
    src/parser/parser_expr.c src/parser/parser_expr_primary.c \
    src/parser/parser_layout.c \
    src/diag/diag.c src/diag/diag_render.c src/diag/diag_render_format.c src/diag/diag_render_source.c \
    src/semantic/types.c src/semantic/symbol.c src/semantic/builtins.c \
    src/semantic/sema.c src/semantic/sema_layout.c src/semantic/sema_generic.c \
    src/semantic/sema_util.c src/semantic/sema_type.c \
    src/semantic/sema_decl.c src/semantic/sema_expr.c src/semantic/sema_stmt.c \
    src/semantic/sema_lit.c src/semantic/sema_call.c \
    src/codegen/codegen.c src/codegen/codegen_emit.c src/codegen/codegen_type.c \
    src/codegen/codegen_overload.c src/codegen/codegen_expr.c src/codegen/codegen_call.c \
    src/codegen/codegen_stmt.c src/codegen/codegen_lambda.c \
    src/codegen/codegen_decl.c src/codegen/codegen_header.c src/codegen/print_fmt.c \
    src/llvm/codegen_llvm.c src/llvm/codegen_llvm_emit.c src/llvm/codegen_llvm_type.c \
    src/llvm/codegen_llvm_expr.c src/llvm/codegen_llvm_stmt.c src/llvm/codegen_llvm_decl.c \
    src/llvm/codegen_llvm_debug.c \
    src/interp/interp.c src/interp/interp_builtin.c src/interp/interp_value.c \
    src/interp/interp_expr.c src/interp/interp_stmt.c \
    src/layout/registry.c src/layout/layout_gen.c src/layout/layout_gen_element.c \
    src/layout/layout_gen_value.c src/layout/schema.c src/layout/layout_expand.c \
    src/fmt/fmt.c src/fmt/fmt_rules.c \
    src/cli/cli.c src/cli/cli_help.c src/cli/cli_options.c src/cli/cli_subcmd.c \
    src/driver/driver.c src/driver/build.c src/driver/llvm_build.c \
    src/driver/llvm_toolchain.c \
    src/driver/layout_build.c src/driver/debug_cmd.c src/driver/repl.c \
    $LDLIBS
echo "built ./salamc with $CC"
