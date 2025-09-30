#include <stage/ast/asts/ast_extern/ast_extern_decl/ast_extern_decl_direct_destroy/ast_extern_decl_direct_destroy.h>

void ast_extern_decl_direct_destroy(ast_extern_decl_t extern_decl) {
    if (extern_decl.value != NULL) {
        ast_destroy(extern_decl.value);
    }
}
