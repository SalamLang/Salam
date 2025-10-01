#include "ast_statement_if_direct_json.h"

char* ast_statement_if_direct_json(ast_statement_if_t statement_if) {
    DEBUG_ME;
    buffer_t* temp = buffer_create(24);

    buffer_append_char(temp, '{');

    // is_last
    buffer_append_str(temp, "\"is_last\":");
    if (statement_if.is_last == true) {
        buffer_append_str(temp, "true");
    } else {
        buffer_append_str(temp, "false");
    }

    // condition
    buffer_append_str(temp, "\"condition\":");
    if (statement_if.condition == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* condition = ast_json(statement_if.condition);
        buffer_append_str(temp, condition);
        memory_destroy(condition);
    }

    // then_branch
    buffer_append_str(temp, "\"then_branch\":");
    if (statement_if.then_branch == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* then_branch = ast_json(statement_if.then_branch);
        buffer_append_str(temp, then_branch);
        memory_destroy(then_branch);
    }

    // else_branch
    buffer_append_str(temp, "\"else_branch\":");
    if (statement_if.else_branch == NULL) {
        buffer_append_str(temp, "null");
    } else {
        char* else_branch = ast_json(statement_if.else_branch);
        buffer_append_str(temp, else_branch);
        memory_destroy(else_branch);
    }

    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
