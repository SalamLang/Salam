#include <utility/file/file_append/file_append.h>

bool file_append(const char* path, const char* content) {
    FILE* file = fopen(path, "a");
    if (file == NULL) return false;
    if (fputs(content, file) == EOF) {
        fclose(file);
        return false;
    }
    fclose(file);
    return true;
}
