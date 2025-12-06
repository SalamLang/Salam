#include "ast_type_name.h"

char* ast_type_name(ast_type_type_t type) {
    switch (type) {
        case AST_KIND_TYPE_INT:
            return "TYPE_INT";
        case AST_KIND_TYPE_FLOAT:
            return "TYPE_FLOAT";
        case AST_KIND_TYPE_SIZE:
            return "TYPE_SIZE";
        case AST_KIND_TYPE_STRING:
            return "TYPE_STRING";
        case AST_KIND_TYPE_BOOL:
            return "TYPE_BOOL";
        case AST_KIND_TYPE_ARRAY:
            return "TYPE_ARRAY";
        case AST_KIND_TYPE_MAP:
            return "TYPE_MAP";
        case AST_KIND_TYPE_SET:
            return "TYPE_SET";
        case AST_KIND_TYPE_TUPLE:
            return "TYPE_TUPLE";
        case AST_KIND_TYPE_VOID:
            return "TYPE_VOID";
        case AST_KIND_TYPE_ANY:
            return "TYPE_ANY";
        case AST_KIND_TYPE_USER_DEFINED:
            return "TYPE_USER_DEFINED";
    }
    return "TYPE_UNKNOWN";
}
