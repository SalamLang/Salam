#include <stage/ast/asts/ast_parameter/ast_parameter_item/ast_parameter_item_destroy/ast_parameter_item_destroy.h>

void ast_parameter_item_destroy(ast_parameter_item_t* parameter_item) {
    DEBUG_ME;
    if (parameter_item->name != NULL) {
        memory_destroy(parameter_item->name);
    }

    if (parameter_item->type != NULL) {
        ast_destroy(parameter_item->type);
    }

    if (parameter_item->default_value != NULL) {
        ast_destroy(parameter_item->default_value);
    }

    memory_destroy(parameter_item);
}
