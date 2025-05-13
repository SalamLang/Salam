#include <stage/parser/parser_log/parser_log.h>

void parser_log(parser_t* parser)
{
    if (!parser) return;

    log_info("Parser Scanner: %p\n", parser->scanner);
    // scanner_log(parser->scanner);
}
