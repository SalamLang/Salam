#include <stage/ast/asts/ast_parameter/ast_parameters/ast_parameters_create/ast_parameters_create.h>

ast_t* ast_parameters_create(array_t* values, size_t value_count)
{
    DEBUG_ME;
    ast_t* parameters = ast_alloc(AST_TYPE_PARAMETERS);
    parameters->raw.parameters_value.values = values;
    parameters->raw.parameters_value.value_count = value_count;
    return parameters;
}
