#include "ast_type_create_from_token.h"

ast_t* ast_type_create_from_token(token_t* token) {
    DEBUG_ME;
    if (!token) return NULL;

    switch (token->type) {
        case TOKEN_TYPE_KIND_INT:
            return ast_type_create_primitive(AST_KIND_TYPE_INT, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_FLOAT:
            return ast_type_create_primitive(AST_KIND_TYPE_FLOAT, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_STRING:
            return ast_type_create_primitive(AST_KIND_TYPE_STRING, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_BOOL:
            return ast_type_create_primitive(AST_KIND_TYPE_BOOL, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_ARRAY:
            return ast_type_create_primitive(AST_KIND_TYPE_ARRAY, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_MAP:
            return ast_type_create_primitive(AST_KIND_TYPE_MAP, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_SET:
            return ast_type_create_primitive(AST_KIND_TYPE_SET, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_TUPLE:
            return ast_type_create_primitive(AST_KIND_TYPE_TUPLE, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_VOID:
            return ast_type_create_primitive(AST_KIND_TYPE_VOID, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_ANY:
            return ast_type_create_primitive(AST_KIND_TYPE_ANY, false, false,
                                             false);
            break;

        case TOKEN_TYPE_KIND_SIZE:
            return ast_type_create_primitive(AST_KIND_TYPE_SIZE, false, false,
                                             false);
            break;

            // TODO: AST_KIND_TYPE_USER_DEFINED as result...

        default:
            break;
    }

    return NULL;
}
