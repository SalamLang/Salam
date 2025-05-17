#include <stage/ast/asts/ast_attribute/ast_attribute_item/ast_attribute_item_direct_json/ast_attribute_item_direct_json.h>

char* ast_attribute_item_direct_json(ast_attribute_item_t attribute_item)
{
    buffer_t* temp = buffer_create(10);

    buffer_append_char(temp, '{');

    // name
    buffer_append_str(temp, "\"name\":");
    if (attribute_item.name == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* name = string_escaping(attribute_item.name);
        buffer_append_str(temp, name);
        memory_destroy(name);
    }

    // value
    buffer_append_str(temp, "\"value\":");
    if (attribute_item.value == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* value = ast_json(attribute_item.value);
        buffer_append_str(temp, value);
        memory_destroy(value);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
