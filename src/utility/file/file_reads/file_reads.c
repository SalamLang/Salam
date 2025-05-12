#include <utility/file/file_reads/file_reads.h>

char* file_reads(const char* file_path)
{
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        log_fatal("Error: cannot open file '%s'\n", file_path);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    char* data = (char*)memory_allocation(file_size + 1);
    size_t read_size = fread(data, 1, file_size, file);
    data[read_size] = '\0';

    fclose(file);
    return data;
}
