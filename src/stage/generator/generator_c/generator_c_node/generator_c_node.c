#include "generator_c_node.h"

buffer_t* generator_c_node(generator_t* generator, ast_t* ast) {
    DEBUG_ME;
    if (ast == NULL) {
        log_fatal("generator_c_node: ast is NULL");
        return NULL;
    }
    log_info("generator_c_node: %s(%d)\n", ast_node_type_name(ast->base.type),
             ast->base.type);
    buffer_t* temp;

    switch (ast->base.type) {
        case AST_TYPE_TYPE:
            DEBUG_ME;
            temp = generator_c_type(generator, ast);
            return temp;
            break;

        case AST_TYPE_PROGRAM:
            DEBUG_ME;
            temp = generator_c_program(generator, ast);
            return temp;
            break;

        case AST_TYPE_BLOCK:
            DEBUG_ME;
            temp = generator_c_block(generator, ast);
            return temp;
            break;

        case AST_TYPE_STATEMENT_PRINT:
            DEBUG_ME;
            temp = generator_c_statement_print(generator, ast);
            return temp;
            break;

        case AST_TYPE_STATEMENT_RAW:
            DEBUG_ME;
            temp = generator_c_statement_raw(generator, ast);
            return temp;
            break;

        case AST_TYPE_STATEMENT_RET:
            DEBUG_ME;
            temp = generator_c_statement_ret(generator, ast);
            return temp;
            break;

        case AST_TYPE_FUNCTION_DECL:
            DEBUG_ME;
            temp = generator_c_function_decl(generator, ast);
            return temp;
            break;

        case AST_TYPE_VARIABLE_DECL:
            DEBUG_ME;
            temp = generator_c_variable_decl(generator, ast);
            return temp;
            break;

        case AST_TYPE_EXPRESSION_ITEM:
            temp = generator_c_expression_item(generator, ast);
            return temp;
            break;

        case AST_TYPE_UNKNOWN:
        case AST_TYPE_PACKAGE:
        case AST_TYPE_IMPORT:
        case AST_TYPE_KIND_DECL:
        case AST_TYPE_KIND_STRUCT:
        case AST_TYPE_KIND_ENUM:
        case AST_TYPE_KIND_UNION:
        case AST_TYPE_EXTERN_DECL:
        case AST_TYPE_EXTERN_VARIABLE:
        case AST_TYPE_EXTERN_FUNCTION:
        case AST_TYPE_PARAMETER_ITEM:
        case AST_TYPE_PARAMETERS:
        case AST_TYPE_ATTRIBUTE_ITEM:
        case AST_TYPE_ATTRIBUTES:
        case AST_TYPE_ARGUMENT_ITEM:
        case AST_TYPE_ARGUMENTS:
        case AST_TYPE_EXPRESSIONS:
        case AST_TYPE_EXPRESSION_LITERAL:
        case AST_TYPE_EXPRESSION_IDENTIFIER:
        case AST_TYPE_EXPRESSION_BINARY:
        case AST_TYPE_EXPRESSION_UNARY:
        case AST_TYPE_EXPRESSION_INDEX:
        case AST_TYPE_EXPRESSION_CALL:
        case AST_TYPE_STATEMENT_IF:
        case AST_TYPE_STATEMENT_FOR:
        case AST_TYPE_STATEMENT_FOREACH:
        case AST_TYPE_STATEMENT_VARIABLE_DECL:
        case AST_TYPE_STATEMENT_EXPRESSION:
            break;
    }

    log_fatal("generator_c_node: unhandled AST type: %d", ast->base.type);
    return NULL;
}
