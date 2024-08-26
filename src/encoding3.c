#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <errno.h>

void print_utf8_chars(FILE *file) {
    unsigned char c;
    while ((c = fgetc(file)) != EOF) {
        if (c <= 0x7F) {
            // ASCII characters (1 byte)
            putchar(c);
        } else if ((c >> 5) == 0x6) {
            // 2-byte UTF-8 sequence
            unsigned char c2 = fgetc(file);
            if (c2 == EOF) {
                perror("Unexpected end of file");
                exit(EXIT_FAILURE);
            }
            printf("%c%c", c, c2);
        } else if ((c >> 4) == 0xE) {
            // 3-byte UTF-8 sequence
            unsigned char c2 = fgetc(file);
            unsigned char c3 = fgetc(file);
            if (c2 == EOF || c3 == EOF) {
                perror("Unexpected end of file");
                exit(EXIT_FAILURE);
            }
            printf("%c%c%c", c, c2, c3);
        } else if ((c >> 3) == 0x1E) {
            // 4-byte UTF-8 sequence
            unsigned char c2 = fgetc(file);
            unsigned char c3 = fgetc(file);
            unsigned char c4 = fgetc(file);
            if (c2 == EOF || c3 == EOF || c4 == EOF) {
                perror("Unexpected end of file");
                exit(EXIT_FAILURE);
            }
            printf("%c%c%c%c", c, c2, c3, c4);
        } else {
            fprintf(stderr, "Unexpected UTF-8 byte sequence");
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    setlocale(LC_CTYPE, "");

    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    print_utf8_chars(file);

    fclose(file);
    return EXIT_SUCCESS;
}
