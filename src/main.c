#include "main.h"

int main(int argc, char** argv)
{
    if (argc < 2) {
        log_fatal("Error: please run ./salam <filename>\n");
    }

    char* data = file_reads(argv[1]);

    for (char* p = data; *p != '\0'; ++p) {
        printf("Char: '%c' (0x%02X)\n", *p, (unsigned char)*p);
    }

    memory_free(data);
    return 0;
}
