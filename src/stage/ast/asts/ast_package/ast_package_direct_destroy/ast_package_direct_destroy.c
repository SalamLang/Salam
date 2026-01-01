#include "ast_package_direct_destroy.h"

void ast_package_direct_destroy(ast_package_t package) {
    DEBUG_ME;
    if (package.name != NULL) {
        memory_destroy(package.name);
    }
}
