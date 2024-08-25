#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

bool file_appends(const char* path, const char* content) {
    FILE* file = fopen(path, "a");
    if (file == NULL) {
        perror("fopen");
        return false;
    }

    size_t size = strlen(content);

    fwrite(content, 1, size, file);

    fclose(file);

    return true;
}

bool file_appends_utf8(const char* path, const unsigned char *bytes, size_t length) {
    return file_appends(path, (const char*)bytes);
}

wchar_t utf8_to_wchar(const unsigned char *bytes, int *bytes_consumed) {
    uint32_t codepoint = 0;
    int length = 0;

    unsigned char c1 = bytes[0];

    if (c1 <= 0x7F) {
        codepoint = c1;
        length = 1;
    } else if ((c1 & 0xE0) == 0xC0) {
        if ((bytes[1] & 0xC0) != 0x80) {
            length = 0;
        } else {
            codepoint = (c1 & 0x1F) << 6;
            codepoint |= (bytes[1] & 0x3F);
            length = 2;
        }
    } else if ((c1 & 0xF0) == 0xE0) {
        if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80) {
            length = 0;
        } else {
            codepoint = (c1 & 0x0F) << 12;
            codepoint |= (bytes[1] & 0x3F) << 6;
            codepoint |= (bytes[2] & 0x3F);
            length = 3;
        }
    } else if ((c1 & 0xF8) == 0xF0) {
        if ((bytes[1] & 0xC0) != 0x80 || (bytes[2] & 0xC0) != 0x80 || (bytes[3] & 0xC0) != 0x80) {
            length = 0;
        } else {
            codepoint = (c1 & 0x07) << 18;
            codepoint |= (bytes[1] & 0x3F) << 12;
            codepoint |= (bytes[2] & 0x3F) << 6;
            codepoint |= (bytes[3] & 0x3F);
            length = 4;
        }
    } else {
        length = 0;
    }

    if (codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
        length = 0;
    }

    *bytes_consumed = length;
    return (wchar_t)codepoint;
}

void print_utf8_as_wchars(const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    setlocale(LC_CTYPE, "");

    unsigned char buffer[4];
    int bytes_consumed;

    while (fread(buffer, 1, 1, file) == 1) {
        if (buffer[0] <= 0x7F) {
            putwchar(buffer[0]);
            file_appends_utf8("windows-logs.txt", buffer, 1);
        } else if ((buffer[0] & 0xE0) == 0xC0) {
            fread(buffer + 1, 1, 1, file);
            file_appends_utf8("windows-logs.txt", buffer, 2);
        } else if ((buffer[0] & 0xF0) == 0xE0) {
            fread(buffer + 1, 1, 2, file);
            file_appends_utf8("windows-logs.txt", buffer, 3);
        } else if ((buffer[0] & 0xF8) == 0xF0) {
            fread(buffer + 1, 1, 3, file);
            file_appends_utf8("windows-logs.txt", buffer, 4);
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        wprintf(L"Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    print_utf8_as_wchars(argv[1]);

    return 0;
}
