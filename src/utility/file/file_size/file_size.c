#include <utility/file/file_size/file_size.h>

long file_size(const char* file_path) {
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        log_fatal("Error: cannot open file '%s'\n", file_path);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);

    fclose(file);
    return file_size;
}
