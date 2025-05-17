#include "ast_import_direct_destroy.h"

void ast_import_direct_destroy(ast_import_t import)
{
    DEBUG_ME;
    if (import.name != NULL) {
        memory_destroy(import.name);
    }
}
