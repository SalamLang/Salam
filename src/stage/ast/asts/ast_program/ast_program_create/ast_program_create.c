#include <stage/ast/asts/ast_program/ast_program_create/ast_program_create.h>

ast_t* ast_program_create(array_t* variable_declarations,
                          array_t* function_declarations,
                          size_t variable_declaration_count,
                          size_t function_declaration_count) {
    ast_t* ast = ast_alloc(AST_TYPE_PROGRAM);
    ast->raw.program_value.variable_declarations = variable_declarations;
    ast->raw.program_value.function_declarations = function_declarations;
    ast->raw.program_value.variable_declaration_count =
        variable_declaration_count;
    ast->raw.program_value.function_declaration_count =
        function_declaration_count;
    return ast;
}
