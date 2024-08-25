#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <iconv.h>
#include <string.h>

#define BUFFER_SIZE 1024

// Function to convert UTF-8 to wide characters
wchar_t* utf8_to_wchar(const char *utf8_text) {
    iconv_t cd = iconv_open("WCHAR_T", "UTF-8");
    if (cd == (iconv_t)-1) {
        perror("iconv_open");
        return NULL;
    }

    size_t inbytesleft = strlen(utf8_text);
    size_t outbytesleft = (inbytesleft + 1) * sizeof(wchar_t);
    wchar_t *wtext = (wchar_t*)malloc(outbytesleft);
    if (!wtext) {
        perror("malloc");
        iconv_close(cd);
        return NULL;
    }

    char *inbuf = (char*)utf8_text;
    char *outbuf = (char*)wtext;

    if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == (size_t)-1) {
        perror("iconv");
        free(wtext);
        iconv_close(cd);
        return NULL;
    }

    *outbuf = L'\0';
    iconv_close(cd);

    return wtext;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        wprintf(L"Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    setlocale(LC_ALL, "");

    FILE *file = fopen(argv[1], "r");
    if (!file) {
        perror("fopen");
        return 1;
    }

    char buffer[BUFFER_SIZE];
    fread(buffer, 1, BUFFER_SIZE - 1, file);
    fclose(file);
    buffer[BUFFER_SIZE - 1] = '\0';  // Null-terminate the buffer

    wchar_t *file_content = utf8_to_wchar(buffer);
    if (!file_content) {
        return 1;
    }

    // Print each wide character one by one
    for (int i = 0; file_content[i] != L'\0'; ++i) {
        wprintf(L"Character: %lc\n", file_content[i]);
    }

    free(file_content);
    return 0;
}
