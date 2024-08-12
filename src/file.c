#include "file.h"

/**
 * 
 * @function file_reads
 * @berif Reading entire of a file
 * @params {char*} path - Path of file
 * @params {size_t*} size - Size of file
 * @returns {char*} - Content of file
 * 
 */
char* file_reads(const char* path, size_t* size)
{
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        panic("Failed to open file");
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    if (size != NULL) {
        *size = file_size;
    }
    fseek(file, 0, SEEK_SET);

    char* content = (char*)memory_allocate(file_size + 1);
    fread(content, 1, file_size, file);
    content[file_size] = '\0';

    fclose(file);

    return content;
}

/**
 * 
 * @function file_writes
 * @berif Writing content to a file
 * @params {char*} path - Path of file
 * @params {char*} content - Content of file
 * @returns {bool}
 * 
 */
bool file_writes(const char* path, const char* content)
{
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        panic("Failed to open file");
    }

    size_t size = strlen(content);

    fwrite(content, 1, size, file);

    fclose(file);

    return true;
}

/**
 * 
 * @function file_exists
 * @berif Check if a file exists
 * @params {char*} path - Path of file
 * @returns {bool}
 * 
 */
bool file_exists(const char* path)
{
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        return false;
    }

    fclose(file);

    return true;
}

/**
 * 
 * @function file_remove
 * @berif Remove a file
 * @params {char*} path - Path of file
 * @returns {bool}
 * 
 */
bool file_remove(const char* path)
{
    if (remove(path) != 0) {
        return false;
    }

    return true;
}

/**
 * 
 * @function file_copy
 * @berif Copy a file
 * @params {char*} source - Source file
 * @params {char*} destination - Destination file
 * @returns {bool}
 * 
 */
bool file_copy(const char* source, const char* destination)
{
    char* content;
    size_t size;

    content = file_reads(source, &size);
    file_writes(destination, content);

    if (content != NULL) {
        memory_destroy(content);
    }

    return true;
}

/**
 * 
 * @function file_move
 * @berif Move a file
 * @params {char*} source - Source file
 * @params {char*} destination - Destination file
 * @returns {bool}
 * 
 */
bool file_move(const char* source, const char* destination)
{
    if (rename(source, destination) != 0) {
        return false;
    }

    return true;
}

/**
 * 
 * @function file_get_name
 * @berif Get the name of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Name of file
 * 
 */
char* file_get_name(const char* path)
{
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last = last_slash > last_backslash ? last_slash : last_backslash;

    if (last == NULL) {
        return (char*)path;
    }

    return (char*)(last + 1);
}

/**
 * 
 * @function file_get_extension
 * @berif Get the extension of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Extension of file
 * 
 */
char* file_get_extension(const char* path)
{
    const char* last_dot = strrchr(path, '.');

    if (last_dot == NULL) {
        return "";
    }

    return (char*)(last_dot + 1);
}

/**
 * 
 * @function file_get_directory
 * @berif Get the directory of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Directory of file
 * 
 */
char* file_get_directory(const char* path)
{
    const char* last_slash = strrchr(path, '/');
    const char* last_backslash = strrchr(path, '\\');
    const char* last = last_slash > last_backslash ? last_slash : last_backslash;

    if (last == NULL) {
        return "";
    }
    
    size_t size = last - path;
    char* directory = (char*)memory_allocate(size + 1);
    strncpy(directory, path, size);
    directory[size] = '\0';

    return directory;
}

/**
 * 
 * @function file_get_absolute
 * @brief Resolves the absolute path of a given relative path.
 * @params {const char*} path - The relative path.
 * @returns {char*} - The resolved absolute path, or NULL if an error occurred.
 * 
 */
char* file_get_absolute(const char* path)
{
    if (!path) {
        return NULL;
    }

    char* resolved_path = memory_allocate(4096);
    if (!resolved_path) {
        return NULL;
    }

    char* ptr = resolved_path;
    const char* src = path;

    if (*src == '/') {
        *ptr++ = '/';
        src++;
    }

    while (*src) {
        while (*src == '/') {
            src++;
        }

        if (!*src) {
            break;
        }

        if (*src == '.' && (src[1] == '/' || src[1] == '\0')) {
            src++;
            continue;
        }

        if (*src == '.' && src[1] == '.' && (src[2] == '/' || src[2] == '\0')) {
            src += 2;

            if (ptr > resolved_path + 1) {
                ptr--;
                while (ptr > resolved_path && *(ptr - 1) != '/') {
                    ptr--;
                }
            }
            continue;
        }

        while (*src && *src != '/') {
            *ptr++ = *src++;
        }

        if (*src) {
            *ptr++ = *src++;
        }
    }

    *ptr = '\0';

    if (resolved_path[0] == '\0') {
        strcpy(resolved_path, "/");
    }

    return resolved_path;
}

/**
 * 
 * @function file_get_size
 * @berif Get the size of a file
 * @params {char*} path - Path of file
 * @returns {long} - Size of file
 * 
 */
long file_get_size(const char* path)
{
    struct stat st;
    stat(path, &st);

    return st.st_size;
}

/**
 * 
 * @function file_get_modified
 * @berif Get the last modified time of a file
 * @params {char*} path - Path of file
 * @returns {time_t} - Last modified time of file
 * 
 */
time_t file_get_modified(const char* path)
{
    struct stat st;
    stat(path, &st);

    return st.st_mtime;
}
