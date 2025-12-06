#include <stage/parser/parser_json/parser_json.h>

char* parser_json(parser_t* parser) {
    buffer_t* temp = buffer_create(24);

    // TODO

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
