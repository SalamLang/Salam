#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <errno.h>
#include <string.h>

// Custom panic function to handle errors
void panic(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Function to read the contents of a file as UTF-8 encoded text
char* file_reads(const char* path, size_t* size) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        panic("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_capacity = ftell(file);
    if (size != NULL) {
        *size = file_capacity;
    }
    fseek(file, 0, SEEK_SET);

    char* content = (char*)malloc(file_capacity + 1);
    if (content == NULL) {
        panic("Memory allocation failed");
    }

    fread(content, 1, file_capacity, file);
    content[file_capacity] = '\0';

    fclose(file);

    return content;
}

// Function to convert a UTF-8 encoded string to a wide string
wchar_t* utf8_to_wide(const char* utf8, size_t length, size_t* wide_length) {
    // Use `mbstowcs` to convert UTF-8 string to wide string
    size_t buf_size = mbstowcs(NULL, utf8, 0);
    if (buf_size == (size_t)-1) {
        panic("Conversion from UTF-8 to wide string failed");
    }

    wchar_t* wide = (wchar_t*)malloc((buf_size + 1) * sizeof(wchar_t));
    if (wide == NULL) {
        panic("Memory allocation failed");
    }

    size_t result = mbstowcs(wide, utf8, buf_size + 1);
    if (result == (size_t)-1) {
        panic("Conversion from UTF-8 to wide string failed");
    }

    if (wide_length != NULL) {
        *wide_length = buf_size;
    }

    return wide;
}

int main(int argc, char** argv) {
    // Ensure the locale is set for wide character printing
    setlocale(LC_ALL, "");

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    size_t size;
    char* utf8_content = file_reads(argv[1], &size);

    if (utf8_content != NULL) {
        size_t wide_size;
        wchar_t* wide_content = utf8_to_wide(utf8_content, size, &wide_size);

        // Print each wchar_t character one by one
        for (size_t i = 0; i < wide_size; i++) {
            wprintf(L"%lc", wide_content[i]);
        }
        wprintf(L"\n");

        // Free allocated memory
        free(utf8_content);
        free(wide_content);
    }

    return 0;
}
