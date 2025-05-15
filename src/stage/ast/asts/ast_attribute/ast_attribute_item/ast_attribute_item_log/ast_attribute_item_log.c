#include <stage/ast/asts/ast_attribute/ast_attribute_item/ast_attribute_item_log/ast_attribute_item_log.h>

void ast_attribute_item_log(ast_attribute_item_t* attribute_item)
{
    if (!attribute_item) {
        log_info("Attribute Item is NULL");
        return;
    }

    log_info("Attribute Item...");
}
