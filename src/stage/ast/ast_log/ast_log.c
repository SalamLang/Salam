#include <stage/ast/ast_log/ast_log.h>

void ast_log(ast_t* ast)
{
    if (ast == NULL) {
        printf("Ast: NULL\n");
        return;
    }

    printf("Ast: ...\n");
}
