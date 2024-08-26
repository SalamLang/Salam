#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_utf8_characters(const char *buffer, size_t size)
{
    size_t i = 0;
    while (i < size) {
        unsigned char ch = buffer[i];
        unsigned char utf8_char[5] = {0};
        utf8_char[0] = ch;

        int num_bytes = 0;
        if ((ch & 0x80) == 0) {
            num_bytes = 1;
        } else if ((ch & 0xE0) == 0xC0) {
            num_bytes = 2;
        } else if ((ch & 0xF0) == 0xE0) {
            num_bytes = 3;
        } else if ((ch & 0xF8) == 0xF0) {
            num_bytes = 4;
        } else {
            fprintf(stderr, "Invalid UTF-8 encoding detected.\n");
            i++;
            continue;
        }

        if (i + num_bytes > size) {
            fprintf(stderr, "Incomplete UTF-8 sequence at the end of buffer.\n");
            break;
        }

        for (int j = 1; j < num_bytes; ++j) {
            unsigned char next_ch = buffer[i + j];
            if ((next_ch & 0xC0) != 0x80) {
                fprintf(stderr, "Invalid UTF-8 sequence detected.\n");
                i += j;
                continue;
            }
            utf8_char[j] = next_ch;
        }

        printf("%s", utf8_char);
        i += num_bytes;
    }
}

char* read_file_into_buffer(const char *filename, size_t *size)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Error opening file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char*)malloc(*size + 1);
    if (buffer == NULL) {
        perror("Memory allocation failed");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, *size, file);
    buffer[*size] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    size_t size = 0;
    char *buffer = read_file_into_buffer(argv[1], &size);
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }

    print_utf8_characters(buffer, size);

    free(buffer);
    return EXIT_SUCCESS;
}
