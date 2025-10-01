#include <utility/file/file_create/file_create.h>

bool file_create(const char* path) {
    FILE* file = fopen(path, "w");
    if (file == NULL) return false;
    fclose(file);
    return true;
}
