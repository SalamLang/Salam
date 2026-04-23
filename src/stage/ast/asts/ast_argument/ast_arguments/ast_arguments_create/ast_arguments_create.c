#include <stage/ast/asts/ast_argument/ast_arguments/ast_arguments_create/ast_arguments_create.h>

ast_t* ast_arguments_create(array_t* values, size_t value_count) {
    DEBUG_ME;
    ast_t* arguments = ast_alloc(AST_TYPE_ARGUMENTS);
    arguments->raw.arguments_value.values = values;
    arguments->raw.arguments_value.value_count = value_count;
    return arguments;
}
