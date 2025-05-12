#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(1);
}

void memory_free(void* ptr)
{
    free(ptr);
}

void* memory_alloc(size_t siz)
{
    void* ptr = malloc(siz);
    if (ptr == NULL) {
        error("Memory allocation failed\n");
    }
    return ptr;
}

char* file_reads(const char* file_path)
{
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        error("Error: cannot open file '%s'\n", file_path);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* data = (char*)memory_alloc(file_size + 1);
    size_t read_size = fread(data, 1, file_size, file);
    data[read_size] = '\0';

    fclose(file);
    return data;
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Error: please run ./salam <filename>\n");
        return 1;
    }

    char* data = file_reads(argv[1]);

    for (char* p = data; *p != '\0'; ++p) {
        printf("Char: '%c' (0x%02X)\n", *p, (unsigned char)*p);
    }

    memory_free(data);
    return 0;
}
