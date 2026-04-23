#include <stage/ast/asts/ast_argument/ast_arguments/ast_arguments_destroy/ast_arguments_destroy.h>

void ast_arguments_destroy(ast_arguments_t* arguments) {
    DEBUG_ME;
    // if (! arguments) return;

    if (arguments->values != NULL) {
        array_destroy(arguments->values);
    }

    memory_destroy(arguments);
}
