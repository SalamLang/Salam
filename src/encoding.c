#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

// include MB_LEN_MAX
#include <limits.h>

// Function to log a wide character to the log file
void log_wchar_t(FILE *log_file, wchar_t wc) {
    if (fwprintf(log_file, L"%lc", wc) < 0) {
        perror("fwprintf");
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_CTYPE, "");

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_path>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    FILE *log_file;
    // #ifdef _WIN32
    //     printf("Windows detected\n");
    //     log_file = _wfopen(L"windows-logs.txt", L"a");
    // #else
        log_file = fopen("windows-logs.txt", "w");
    // #endif

    if (!log_file) {
        perror("fopen log file");
        fclose(file);
        return 1;
    }

    char buffer[MB_LEN_MAX + 1];
    int buffer_index = 0;
    wchar_t wc;
    int bytes_read;

    while ((bytes_read = fread(buffer + buffer_index, 1, 1, file)) > 0) {
        buffer[buffer_index + 1] = '\0';

        mbstate_t state = {0};
        size_t result = mbrtowc(&wc, buffer, buffer_index + 1, &state);

        if (result == (size_t)-1 || result == (size_t)-2) {
            buffer_index++;
            if (buffer_index >= MB_LEN_MAX) {
                fprintf(stderr, "Invalid UTF-8 sequence detected.\n");
                break;
            }
        } else {
            wprintf(L"%lc", wc);
            log_wchar_t(log_file, wc);
            buffer_index = 0;
        }
    }

    if (ferror(file)) {
        perror("fread");
    }

    fclose(file);
    fclose(log_file);

    return 0;
}
