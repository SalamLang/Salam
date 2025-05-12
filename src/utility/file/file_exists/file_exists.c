#include <utility/file/file_exists/file_exists.h>

bool file_exists(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}
