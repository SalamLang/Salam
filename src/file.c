/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 * 
 * Welcome to the Salam Programming Language! Salam is a modern, efficient, 
 * and developer-friendly language created to inspire and empower programmers 
 * of all ages. With its unique features and Persian roots, Salam aims to make 
 * programming more accessible and enjoyable for everyone.
 * 
 * This file is part of the core implementation of Salam, including its runtime 
 * and compiler components. By contributing to or using this codebase, you are 
 * part of a growing community dedicated to innovation and inclusivity in 
 * programming.
 * 
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 * 
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

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
char *file_reads(const char *path, size_t *size) {
    DEBUG_ME;
    // ccs=UTF-8
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        printf("Failed to read file %s\n", path);
        panic("");

        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_capacity = ftell(file);
    if (size != NULL) {
        *size = file_capacity;
    }
    fseek(file, 0, SEEK_SET);

    char *content = (char *)memory_allocate(file_capacity + 1);
    if (content == NULL) {
        fclose(file);
        printf("Failed to allocate memory for file content\n");
        panic("");
        return NULL;
    }

    size_t read_size = fread(content, 1, file_capacity, file);
    if (read_size != file_capacity) {
        fclose(file);
        free(content);
        printf("Failed to read the entire file content\n");
        panic("");
        return NULL;
    }

    content[file_capacity] = '\0';
    fclose(file);

    return content;
}

/**
 *
 * @function file_reads_binary
 * @brief Reading entire of a binary file
 * @params {char*} path - Path of file
 * @params {size_t*} size - Size of file
 * @returns {char*} - Content of file
 *
 */
char *file_reads_binary(const char *path, size_t *size) {
    DEBUG_ME;
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        printf("Failed to open file %s\n", path);
        panic("");

        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t file_capacity = ftell(file);
    if (size != NULL) {
        *size = file_capacity;
    }
    fseek(file, 0, SEEK_SET);

    char *content = (char *)memory_allocate(file_capacity + 1);
    if (content == NULL) {
        fclose(file);
        printf("Failed to allocate memory for file content\n");
        panic("");
        return NULL;
    }

    size_t read_size = fread(content, 1, file_capacity, file);
    if (read_size != file_capacity) {
        fclose(file);
        free(content);
        printf("Failed to read the entire file content\n");
        panic("");
        return NULL;
    }

    content[file_capacity] = '\0';
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
bool file_writes(const char *path, const char *content) {
    DEBUG_ME;
    // ccs=UTF-8
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        printf("Failed to write file %s\n", path);
        panic("");

        return false;
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
bool file_exists(const char *path) {
    DEBUG_ME;
    // ccs=UTF-8
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return false;
    }

    fclose(file);

    return true;
}

/**
 *
 * @function directory_create
 * @berif Create a directory
 * @params {char*} path - Path of directory
 * @returns {bool}
 *
 */
bool directory_exists(const char *path) {
    DEBUG_ME;
    struct stat st;

    if (stat(path, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            return true;
        }
    }

    return false;
}

/**
 *
 * @function file_remove
 * @berif Remove a file
 * @params {char*} path - Path of file
 * @returns {bool}
 *
 */
bool file_remove(const char *path) {
    DEBUG_ME;
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
bool file_copy(const char *source, const char *destination) {
    DEBUG_ME;
    char *content;
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
bool file_move(const char *source, const char *destination) {
    DEBUG_ME;
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
char *file_get_name(const char *path) {
    DEBUG_ME;
    const char *last_slash = strrchr(path, '/');
    const char *last_backslash = strrchr(path, '\\');
    const char *last =
        last_slash > last_backslash ? last_slash : last_backslash;

    if (last == NULL) {
        return (char *)path;
    }

    return (char *)(last + 1);
}

/**
 *
 * @function file_get_extension
 * @berif Get the extension of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Extension of file
 *
 */
char *file_get_extension(const char *path) {
    DEBUG_ME;
    const char *last_dot = strrchr(path, '.');

    if (last_dot == NULL) {
        return "";
    }

    return (char *)(last_dot + 1);
}

/**
 *
 * @function file_get_directory
 * @berif Get the directory of a file
 * @params {char*} path - Path of file
 * @returns {char*} - Directory of file
 *
 */
char *file_get_directory(const char *path) {
    DEBUG_ME;
    const char *last_slash = strrchr(path, '/');
    const char *last_backslash = strrchr(path, '\\');
    const char *last =
        last_slash > last_backslash ? last_slash : last_backslash;

    if (last == NULL) {
        return "";
    }

    size_t size = last - path;
    char *directory = memory_allocate(size + 1);
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
char *file_get_absolute(const char *path) {
    DEBUG_ME;
    if (!path) {
        return NULL;
    }

    char *resolved_path = memory_allocate(4096);
    if (!resolved_path) {
        return NULL;
    }

    char *ptr = resolved_path;
    const char *src = path;

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
 * @function file_get_capacity
 * @berif Get the size of a file
 * @params {char*} path - Path of file
 * @returns {long} - Size of file
 *
 */
long file_get_capacity(const char *path) {
    DEBUG_ME;
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
time_t file_get_modified(const char *path) {
    DEBUG_ME;
    struct stat st;
    stat(path, &st);

    return st.st_mtime;
}

/**
 *
 * @function file_appends
 * @brief Appending content to a file
 * @params {char*} path - Path of file
 * @params {char*} content - Content of file
 * @returns {bool}
 *
 */
bool file_appends(const char *path, const char *content) {
    DEBUG_ME;
    FILE *file = fopen(path, "a");
    if (file == NULL) {
        printf("Failed to append file %s\n", path);
        panic("");

        return false;
    }

    size_t size = strlen(content);

    fwrite(content, 1, size, file);

    fclose(file);

    return true;
}

/**
 *
 * @function file_appends_wchar
 * @brief Appending wide character to a file
 * @params {char*} path - Path of file
 * @params {wchar_t} wc - Wide character
 * @returns {bool}
 *
 */
bool file_appends_wchar(const char *path, const wchar_t wc) {
    char mb_str[16];
    memset(mb_str, 0, sizeof(mb_str));

    size_t len = wcrtomb(mb_str, wc, NULL);
    if (len == (size_t)-1) {
        fprintf(stderr, "Failed to convert wide character\n");

        return false;
    }

    char error_message[256];
    snprintf(error_message, sizeof(error_message), "character '%s'\n", mb_str);

    return file_appends(path, error_message);
}
