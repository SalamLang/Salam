#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

void log_wchar_t(FILE *log_file, wchar_t wc) {
    if (fwprintf(log_file, L"%lc", wc) < 0)
    {
        perror("fwprintf");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file_path>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    FILE *log_file = _wfopen(L"windows-logs.txt", L"w");
    if (!log_file) {
        perror("fopen log file");
        fclose(file);
        return 1;
    }

    wchar_t wc;
    while ((wc = fgetwc(file)) != WEOF) {
        wprintf(L"%lc", wc);

        log_wchar_t(log_file, wc);
    }

    if (ferror(file)) {
        perror("fgetwc");
    }

    fclose(file);
    fclose(log_file);

    return 0;
}
