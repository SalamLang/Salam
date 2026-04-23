#include <stage/ast/asts/ast_attribute/ast_attributes/ast_attributes_direct_destroy/ast_attributes_direct_destroy.h>

void ast_attributes_direct_destroy(ast_attributes_t attributes) {
    DEBUG_ME;
    if (attributes.values != NULL) {
        array_destroy(attributes.values);
    }
}
