#include "generator_c_node.h"

buffer_t* generator_c_node(generator_t* generator, ast_t* ast)
{
    DEBUG_ME;
    log_info("generator_c_node: %s(%d)\n", ast_node_type_name(ast->base.type), ast->base.type);
    buffer_t* temp;

    switch (ast->base.type)
    {
        case AST_TYPE_PROGRAM:
            temp = generator_c_program(generator, ast);
            return temp;
            break;

        case AST_TYPE_BLOCK:
            temp = generator_c_block(generator, ast);
            return temp;
            break;

        case AST_TYPE_STATEMENT_PRINT:
            temp = generator_c_statement_print(generator, ast);
            return temp;
            break;

        case AST_TYPE_STATEMENT_RET:
            temp = generator_c_statement_ret(generator, ast);
            return temp;
            break;

        case AST_TYPE_FUNCTION_DECL:
            temp = generator_c_function_decl(generator, ast);
            return temp;
            break;

        case AST_TYPE_VARIABLE_DECL:
            temp = generator_c_variable_decl(generator, ast);
            return temp;
            break;

        default:
            break;
    }

    log_fatal("generator_c_node: unhandled AST type: %d", ast->base.type);
    return NULL;
}
