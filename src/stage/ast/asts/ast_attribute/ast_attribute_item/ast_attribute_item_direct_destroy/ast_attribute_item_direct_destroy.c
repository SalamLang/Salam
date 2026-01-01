#include <stage/ast/asts/ast_attribute/ast_attribute_item/ast_attribute_item_direct_destroy/ast_attribute_item_direct_destroy.h>

void ast_attribute_item_direct_destroy(ast_attribute_item_t attribute_item) {
    if (attribute_item.name != NULL) {
        memory_destroy(attribute_item.name);
    }

    if (attribute_item.value != NULL) {
        ast_destroy(attribute_item.value);
    }
}
