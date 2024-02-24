#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>

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

bool is_number(wchar_t ch)
{
    return ch >= L'۰' && ch <= L'۹';
}

bool is_alpha(wchar_t ch)
{
    return ch >= L'آ' && ch <= L'ی' || ch == L'_';
}

bool is_ident(wchar_t ch)
{
    return is_alpha(ch) || is_number(ch);
}

wchar_t read_token(lexer_t* lexer)
{
    wchar_t current_char;
    int char_size = mbtowc(&current_char, &lexer->data[lexer->index], MB_CUR_MAX);
    if (char_size < 0) {
        printf("Syntax Error: invalid unicode character\n");
        exit(1);
        return 0;
    }

    if (current_char == '\n') {
        lexer->line++;
        lexer->column = 0;
    } else {
        lexer->column += char_size;
    }

    lexer->index += char_size;

    return current_char;
}

void read_number(lexer_t* lexer, wchar_t ch)
{
    char* number = (char*)malloc(100);
    int i = 0;
    while (is_number(ch)) {
        number[i++] = ch - L'۰' + '0';
        ch = read_token(lexer);
    }
    number[i] = 0;
    printf("number = %s\n", number);
}

void read_identifier(lexer_t* lexer, wchar_t ch)
{
    char identifier[256];
    int i = 0;
    while (is_ident(ch)) {
        int char_size = wctomb(&identifier[i], ch);
        if (char_size < 0) {
            printf("Error: Failed to convert wide character to multibyte\n");
            exit(1);
        }
        i += char_size;
        ch = read_token(lexer);
    }
    identifier[i] = 0;
    printf("identifier = %s\n", identifier);
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

        wchar_t current_char = read_token(lexer);

        // check if current_char is ۱۲۳۴۵۶۷۸۹۰
        if (is_number(current_char)) {
            read_number(lexer, current_char);
        } else if (is_alpha(current_char)) {
            read_identifier(lexer, current_char);
        }
         else {
            printf("character: %lc\n", current_char);
        }
    }
}

void help()
{
    printf("Welcome to Sallam Programming Language!\n");
    printf("Sallam is the first Persian/Iranian computer scripting language.\n");
    printf("\n");

    printf("Usage:\n");
    printf("  sallam <filename>\t\t\t# Execute a Sallam script\n");
    printf("\n");

    printf("Example:\n");
    printf("  sallam my_script.sallam\t\t# Run the Sallam script 'my_script.sallam'\n");
    printf("\n");

    printf("Feel free to explore and create using Sallam!\n");
    printf("For more information, visit: https://sallam-lang.js.org\n");
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    if (argc == 1 || argc > 2) {
        help();
        return 0;
    }

    char* file_data = file_read(argv[1]);
    printf("%s\n", file_data);

    lexer_t* lexer = lexer_create(file_data);
    lexer_lex(lexer);

    printf("lexer has been done\n");

    return 0;
}
