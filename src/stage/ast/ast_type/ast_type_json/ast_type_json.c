#include <stage/ast/ast_type/ast_type_json/ast_type_json.h>

char* ast_type_json(ast_type_t* type)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
    if (type == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // type
        buffer_append_str(temp, "\"type\":");
        buffer_append_str(temp, ast_type_name(type->type));

        // is_pointer
        buffer_append_str(temp, ",\"is_pointer\":");
        buffer_append_str(temp, type->is_pointer ? "true" : "false");

        // is_reference
        buffer_append_str(temp, ",\"is_reference\":");
        buffer_append_str(temp, type->is_reference ? "true" : "false");

        // is_const
        buffer_append_str(temp, ",\"is_const\":");
        buffer_append_str(temp, type->is_const ? "true" : "false");

        // name
        buffer_append_str(temp, ",\"name\":");
        if (type->name == NULL) {
            buffer_append_str(temp, "null");
        } else {
            buffer_append_char(temp, '"');
            char* name = string_escaping(type->name):
            buffer_append_str(temp, name);
            memory_destroy(name);
            buffer_append_char(temp, '"');
        }

        // element_type
        buffer_append_str(temp, ",\"element_type\":");
        if (type->element_type == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* element_type = ast_json(type->element_type);
            buffer_append_str(temp, element_type);
            memory_destroy(element_type);
        }

        // key_type
        buffer_append_str(temp, ",\"key_type\":");
        if (type->key_type == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* key_type = ast_json(type->key_type);
            buffer_append_str(temp, key_type);
            memory_destroy(key_type);
        }

        // value_type
        buffer_append_str(temp, ",\"value_type\":");
        if (type->value_type == NULL) {
            buffer_append_str(temp, "null");
        } else {
            char* value_type = ast_json(type->value_type);
            buffer_append_str(temp, value_type);
            memory_destroy(value_type);
        }

        // tuple_elements
        buffer_append_str(temp, ",\"tuple_elements\":");
        if (type->tuple_elements == NULL) {
            buffer_append_str(temp, "null");
        } else {
            for (size_t i = 0; i < type->tuple_elements->size; i++) {
                if (i > 0) {
                    buffer_append_char(temp, ',');
                }
                ast_t* tuple_element = array_get(type->tuple_elements, i);
                char* tuple_element_json = ast_json(tuple_element);
                buffer_append_str(temp, tuple_element_json);
                memory_destroy(tuple_element_json);
            }
        }

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
