#include "main.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        log_fatal("Error: please run ./salam <filename>\n");
    }
    scanner_t* scanner = scanner_create();
    scanner->source = file_reads(argv[1]);
    scanner_log(scanner);
    scanner_free(scanner);
    return 0;
}
