#include <stage/ast/asts/ast_attribute/ast_attribute_item/ast_attribute_item_create/ast_attribute_item_create.h>

ast_t* ast_attribute_item_create(char* name, ast_t* value)
{
    ast_t* ast = ast_alloc(AST_TYPE_ATTRIBUTE_ITEM);
    ast->raw.attribute_item_value.name = name;
    ast->raw.attribute_item_value.value = value;
    return ast;
}
