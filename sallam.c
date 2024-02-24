#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>

char* file_read(char* file_Name)
{
    FILE* file = fopen(file_Name, "r");
    if (file == NULL)
    {
        printf("Error: file not found\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* file_data = (char*)malloc(file_size + 1);
    fread(file_data, 1, file_size, file);
    file_data[file_size] = 0;

    fclose(file);
    return file_data;
}

typedef enum {
    TOKEN_TYPE_EOF,
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_FUNCTION,
    TOKEN_TYPE_RETURN,
    TOKEN_TYPE_ERROR
} token_type_t;

typedef struct {
    token_type_t type;
    char* value;
    struct {
        int length;
        int line;
        int column;
        int end_line;
        int end_column;
    } location;
} token_t;

typedef struct {
    char* data;
    int index;
    int line;
    int column;
} lexer_t;

lexer_t* lexer_create(const char* data)
{
    lexer_t* lexer = (lexer_t*)malloc(sizeof(lexer_t));
    lexer->data = (char*) data;
    lexer->index = 0;
    return lexer;
}

void lexer_lex(lexer_t* lexer)
{
    printf("lexer_lex\n");
    printf("lexer->data = %s\n", lexer->data);
    printf("lexer->index = %d\n", lexer->index);

    while (lexer->data[lexer->index] != 0) {
        if (lexer->data[lexer->index] == '\a' || lexer->data[lexer->index] == '\r') {
            lexer->column++;
            lexer->index++;
            continue;
        } else if (lexer->data[lexer->index] == ' ' || lexer->data[lexer->index] == '\t') {
            lexer->column++;
            lexer->index++;
            continue;
        } else if (lexer->data[lexer->index] == '\n') {
            lexer->index++;
            lexer->line++;
            lexer->column = 0;
            continue;
        }

        wchar_t current_char;
        int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);

        printf("Character: %lc\n", current_char);
        lexer->index += char_size;
    }
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    printf("Sallam\n");
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    printf("سلام\n");

    char* file_data = file_read("input.sallam");
    printf("%s\n", file_data);

    lexer_t* lexer = lexer_create(file_data);
    lexer_lex(lexer);

    printf("lexer has been done\n");

    return 0;
}
