#include <stage/ast/ast_json/ast_json.h>

char* ast_json(ast_t* ast)
{
    buffer_t* temp = buffer_create(256);
    if (ast == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_str(temp, "AST");
        // buffer_append_str(temp, "{\n");
        // buffer_append_str(temp, "  \"type\": \"ast\",\n");
        // buffer_append_str(temp, "  \"declarations\": [\n");

        // for (size_t i = 0; i < ast->declaration_count; i++) {
        //     if (i > 0) {
        //         buffer_append_str(temp, ",\n");
        //     }
        //     char* json_decl = ast_json(ast->declarations[i]);
        //     buffer_append_str(temp, json_decl);
        //     memory_destroy(json_decl);
        // }

        // buffer_append_str(temp, "\n  ]\n");
        // buffer_append_str(temp, "}");
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
