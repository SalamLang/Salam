#include <utility/file/file_delete/file_delete.h>

bool file_delete(const char* path) {
    // TODO: supporting Windows, POSIX, and MacOS...
    return remove(path) == 0;
}
