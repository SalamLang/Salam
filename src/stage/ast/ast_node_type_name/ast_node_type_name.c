#include "ast_node_type_name.h"

char* ast_node_type_name(ast_node_type_t type)
{
    switch (type) {
        case AST_TYPE_UNKNOWN: return "AST_TYPE_UNKNOWN";
        case AST_TYPE_PROGRAM: return "AST_TYPE_PROGRAM";
        case AST_TYPE_PACKAGE: return "AST_TYPE_PACKAGE";
        case AST_TYPE_IMPORT: return "AST_TYPE_IMPORT";

        // kind
        case AST_TYPE_KIND_DECL: return "AST_TYPE_KIND_DECL";
        case AST_TYPE_KIND_STRUCT: return "AST_TYPE_KIND_STRUCT";
        case AST_TYPE_KIND_ENUM: return "AST_TYPE_KIND_ENUM";
        case AST_TYPE_KIND_UNION: return "AST_TYPE_KIND_UNION";

        // extern
        case AST_TYPE_EXTERN_DECL: return "AST_TYPE_EXTERN_DECL";
        case AST_TYPE_EXTERN_VARIABLE: return "AST_TYPE_EXTERN_VARIABLE";
        case AST_TYPE_EXTERN_FUNCTION: return "AST_TYPE_EXTERN_FUNCTION";

        case AST_TYPE_VARIABLE_DECL: return "AST_TYPE_VARIABLE_DECL";
        case AST_TYPE_FUNCTION_DECL: return "AST_TYPE_FUNCTION_DECL";

        case AST_TYPE_BLOCK: return "AST_TYPE_BLOCK";

        // parameters
        case AST_TYPE_PARAMETER_ITEM: return "AST_TYPE_PARAMETER_ITEM";
        case AST_TYPE_PARAMETERS: return "AST_TYPE_PARAMETERS";

        // attributes
        case AST_TYPE_ATTRIBUTE_ITEM: return "AST_TYPE_ATTRIBUTE_ITEM";
        case AST_TYPE_ATTRIBUTES: return "AST_TYPE_ATTRIBUTES";

        // arguments
        case AST_TYPE_ARGUMENT_ITEM: return "AST_TYPE_ARGUMENT_ITEM";
        case AST_TYPE_ARGUMENTS: return "AST_TYPE_ARGUMENTS";

        // expressions
        case AST_TYPE_EXPRESSION_ITEM: return "AST_TYPE_EXPRESSION_ITEM";
        case AST_TYPE_EXPRESSIONS: return "AST_TYPE_EXPRESSIONS";
        case AST_TYPE_EXPRESSION_LITERAL: return "AST_TYPE_EXPRESSION_LITERAL";
        case AST_TYPE_EXPRESSION_IDENTIFIER: return "AST_TYPE_EXPRESSION_IDENTIFIER";
        case AST_TYPE_EXPRESSION_BINARY: return "AST_TYPE_EXPRESSION_BINARY";
        case AST_TYPE_EXPRESSION_UNARY: return "AST_TYPE_EXPRESSION_UNARY";
        case AST_TYPE_EXPRESSION_INDEX: return "AST_TYPE_EXPRESSION_INDEX";
        case AST_TYPE_EXPRESSION_CALL: return "AST_TYPE_EXPRESSION_CALL";

        // statements
        case AST_TYPE_STATEMENT_IF: return "AST_TYPE_STATEMENT_IF";
        case AST_TYPE_STATEMENT_FOR: return "AST_TYPE_STATEMENT_FOR";
        case AST_TYPE_STATEMENT_FOREACH: return "AST_TYPE_STATEMENT_FOREACH";
        case AST_TYPE_STATEMENT_RET: return "AST_TYPE_STATEMENT_RET";
        case AST_TYPE_STATEMENT_PRINT: return "AST_TYPE_STATEMENT_PRINT";
        case AST_TYPE_STATEMENT_VARIABLE_DECL: return "AST_TYPE_STATEMENT_VARIABLE_DECL";
        case AST_TYPE_STATEMENT_EXPRESSION: return "AST_TYPE_STATEMENT_EXPRESSION";

        default: return "UNKNOWN_AST_TYPE";
    }
}
