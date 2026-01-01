#include <stage/ast/asts/ast_parameter/ast_parameters/ast_parameters_direct_destroy/ast_parameters_direct_destroy.h>

void ast_parameters_direct_destroy(ast_parameters_t parameters) {
    DEBUG_ME;
    if (parameters.values != NULL) {
        array_destroy(parameters.values);
    }
}
