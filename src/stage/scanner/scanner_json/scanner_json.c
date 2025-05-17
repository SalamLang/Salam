#include <stage/scanner/scanner_json/scanner_json.h>

char* scanner_json(scanner_t* scanner)
{
    DEBUG_ME;
    buffer_t* temp = buffer_create(256);
    if (scanner == NULL) {
        buffer_append_str(temp, "null");
    }
    else {
        buffer_append_char(temp, '{');

        // source
        buffer_append_str(temp, "\"source\": ");
        if (scanner->source) {
            buffer_append_char(temp, '"');
            char* scanner_source = string_escaping(scanner->source);
            buffer_append_str(temp, scanner_source);
            memory_destroy(scanner_source);
            buffer_append_char(temp, '"');
        } else {
            buffer_append_str(temp, "null");
        }
        buffer_append_str(temp, ", ");

        // directory
        buffer_append_str(temp, "\"directory\": ");
        if (scanner->directory) {
            buffer_append_char(temp, '"');
            char* scanner_directory = string_escaping(scanner->directory);
            buffer_append_str(temp, scanner_directory);
            memory_destroy(scanner_directory);
            buffer_append_char(temp, '"');
        } else {
            buffer_append_str(temp, "null");
        }
        buffer_append_str(temp, ", ");

        // file_name
        buffer_append_str(temp, "\"file_name\": ");
        if (scanner->file_name) {
            buffer_append_char(temp, '"');
            char* scanner_file_name = string_escaping(scanner->file_name);
            buffer_append_str(temp, scanner_file_name);
            memory_destroy(scanner_file_name);
            buffer_append_char(temp, '"');
        } else {
            buffer_append_str(temp, "null");
        }
        buffer_append_str(temp, ", ");

        // tokens
        size_t token_count = array_size(scanner->tokens);
        buffer_append_str(temp, "\"tokens\": [");
        for (size_t i = 0; i < token_count; ++i) {
            token_t* token = array_get(scanner->tokens, i);
            char* json_token = token_json(token);
            buffer_append_str(temp, json_token);
            memory_destroy(json_token);
            if (i < token_count - 1) {
                buffer_append_str(temp, ", ");
            }
        }
        buffer_append_str(temp, "]");

        buffer_append_char(temp, '}');
    }

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
