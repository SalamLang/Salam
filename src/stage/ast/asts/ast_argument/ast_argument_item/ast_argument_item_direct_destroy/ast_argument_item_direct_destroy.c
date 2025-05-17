#include <stage/ast/asts/ast_argument/ast_argument_item/ast_argument_item_direct_destroy/ast_argument_item_direct_destroy.h>

void ast_argument_item_direct_destroy(ast_argument_item_t argument_item)
{
    DEBUG_ME;
    if (argument_item.value != NULL) {
        ast_destroy(argument_item.value);
    }
}
