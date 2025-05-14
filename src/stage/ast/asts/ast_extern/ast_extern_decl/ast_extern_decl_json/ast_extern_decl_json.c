#include <stage/ast/asts/ast_extern/ast_extern_decl/ast_extern_decl_json/ast_extern_decl_json.h>

char* ast_extern_decl_json(ast_extern_decl_t* extern_decl)
{
    buffer_t* temp = buffer_create(10);

    if (extern_decl == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');
        buffer_append_str(temp, "\"type\":");
        buffer_append_str(temp, ast_extern_type_name(extern_decl->type));
        buffer_append_str(temp, "\"value\":");

        char* value = ast_json(extern_decl->value);
        buffer_append_str(temp, value);
        memory_destroy(value);

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
