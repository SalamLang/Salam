#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <locale.h>
#include <wchar.h>
#include <string.h>

// Function to decode a single UTF-8 byte sequence to its Unicode code point
int utf8_to_codepoint(const char *utf8, int *codepoint) {
    unsigned char byte1 = (unsigned char)utf8[0];

    if (byte1 <= 0x7F) {
        *codepoint = byte1;
        return 1;
    } else if ((byte1 & 0xE0) == 0xC0) {
        if ((utf8[1] & 0xC0) != 0x80) return -1; // Invalid sequence
        *codepoint = ((byte1 & 0x1F) << 6) | (utf8[1] & 0x3F);
        return 2;
    } else if ((byte1 & 0xF0) == 0xE0) {
        if ((utf8[1] & 0xC0) != 0x80 || (utf8[2] & 0xC0) != 0x80) return -1; // Invalid sequence
        *codepoint = ((byte1 & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
        return 3;
    } else if ((byte1 & 0xF8) == 0xF0) {
        if ((utf8[1] & 0xC0) != 0x80 || (utf8[2] & 0xC0) != 0x80 || (utf8[3] & 0xC0) != 0x80) return -1; // Invalid sequence
        *codepoint = ((byte1 & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
        return 4;
    }
    return -1; // Invalid UTF-8 sequence
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_path>\n", argv[0]);
        return 1;
    }

    // Set locale for wide character support
    setlocale(LC_CTYPE, "");

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("fopen");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_contents = (char *)malloc(file_size + 1);
    if (!file_contents) {
        perror("malloc");
        fclose(file);
        return 1;
    }

    fread(file_contents, 1, file_size, file);
    file_contents[file_size] = '\0';

    fclose(file);

    // Allocate an array to store UTF-8 code points
    int *codepoints = (int *)malloc(file_size * sizeof(int));
    if (!codepoints) {
        perror("malloc");
        free(file_contents);
        return 1;
    }

    FILE *log_file = fopen("utf8-log.txt", "w");
    if (!log_file) {
        perror("fopen log file");
        free(file_contents);
        free(codepoints);
        return 1;
    }

    char *ptr = file_contents;
    int index = 0;
    int codepoint;
    while (*ptr) {
        int bytes = utf8_to_codepoint(ptr, &codepoint);
        if (bytes > 0) {
            codepoints[index++] = codepoint;
            ptr += bytes;
        } else {
            fprintf(stderr, "Invalid UTF-8 sequence encountered.\n");
            break;
        }
    }

    // Write code points and their wchar_t representation to the log file
    for (size_t i = 0; i < index; i++) {
        // Convert codepoint to wchar_t
        wchar_t wc = (wchar_t)codepoints[i];
        fprintf(log_file, "Codepoint: U+%04X, wchar_t: %lc\n", codepoints[i], wc);
    }

    free(file_contents);
    free(codepoints);
    fclose(log_file);

    return 0;
}
