#include <utility/file/file_write/file_write.h>

bool file_write(const char* path, const char* content)
{
    FILE* file = fopen(path, "w");
    if (file == NULL) return false;
    if (fputs(content, file) == EOF) {
        fclose(file);
        return false;
    }
    fclose(file);
    return true;
}

