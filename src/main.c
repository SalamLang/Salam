#include "main.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        log_fatal("Error: please run ./salam <filename>\n");
    }
    scanner_t* scanner = scanner_create();
    char* directory = io_directory();
    if (!file_exists(argv[1])) {
        log_fatal("Error: file '%s' does not exist\n", argv[1]);
    }
    char* source = file_reads(argv[1]);
    scanner_set(scanner, source, directory, argv[1]);
    scanner_scan(scanner);
    scanner_log(scanner);
    scanner_free(scanner);
    return 0;
}
