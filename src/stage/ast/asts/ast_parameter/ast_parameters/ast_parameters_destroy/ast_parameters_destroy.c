#include <stage/ast/asts/ast_parameter/ast_parameters/ast_parameters_destroy/ast_parameters_destroy.h>

void ast_parameters_destroy(ast_parameters_t* parameters)
{
    DEBUG_ME;
    // if (! parameters) return;

    if (parameters->values != NULL) {
        array_destroy(parameters->values);
    }

    memory_destroy(parameters);
}
