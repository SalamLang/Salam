#include "generator_c_statement_print.h"

buffer_t* generator_c_statement_print(generator_t* generator, ast_t* ast) {
    DEBUG_ME;
    ast_statement_print_t print = ast->raw.statement_print_value;
    buffer_t* temp = buffer_create(1024);

    buffer_t* ident = generator_c_ident(generator);
    buffer_append(temp, ident);

    buffer_append_str(temp, "printf(");

    buffer_append_char(temp, '\"');
    for (size_t i = 0; i < print.value_count; i++) {
        ast_t* expression_item =
            array_get(print.values->raw.expressions_value.values, i);
        switch (expression_item->raw.expression_item_value.runtime_type->type) {
            case AST_KIND_TYPE_INT:
                buffer_append_str(temp, "%d");
                break;
            case AST_KIND_TYPE_FLOAT:
                buffer_append_str(temp, "%f");
                break;
            case AST_KIND_TYPE_SIZE:
                buffer_append_str(temp, "%zu");
                break;
            case AST_KIND_TYPE_STRING:
                buffer_append_str(temp, "%s");
                break;
            case AST_KIND_TYPE_BOOL:
                buffer_append_str(temp, "%s");
                break;
            default:
                buffer_append_str(temp, "%s");
                break;
        }
    }
    buffer_append_char(temp, '\"');

    buffer_append_str(temp, ", ");

    buffer_t* expressions = generator_c_expressions(generator, print.values);
    buffer_append(temp, expressions);
    buffer_destroy(expressions);

    buffer_append_str(temp, ");\n");

    buffer_destroy(ident);

    return temp;
}
