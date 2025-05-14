#include <stage/parser/parser_create/parser_create.h>

parser_t* parser_create(const scanner_t* scanner)
{
    DEBUG_ME;
    parser_t* parser = memory_allocation(sizeof(parser_t));
    parser->scanner = scanner;
    parser->index = 0;
    return parser;
}
