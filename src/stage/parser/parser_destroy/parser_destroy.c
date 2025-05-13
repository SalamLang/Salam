#include <stage/parser/parser_destroy/parser_destroy.h>

void parser_destroy(parser_t* parser)
{
    // if (!parser) return;

    memory_destroy(parser);
}
