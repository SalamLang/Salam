#include <stage/token/token_location_json/token_location_json.h>

char* token_location_json(token_location_t location)
{
    buffer_t *temp = buffer_create(256);

    buffer_append_char(temp, '{');

    // base
    buffer_append_str(temp, "\"base\": \"token_location_t\",");

    // begin_line
    buffer_append_str(temp, "\"begin_line\": ");
    buffer_append_str(temp, convert_size2string(location.begin_line));

    // begin_column
    buffer_append_str(temp, ", \"begin_column\": ");
    buffer_append_str(temp, convert_size2string(location.begin_column));

    // begin_index
    buffer_append_str(temp, ", \"begin_index\": ");
    buffer_append_str(temp, convert_size2string(location.begin_index));

    // end_line
    buffer_append_str(temp, ", \"end_line\": ");
    buffer_append_str(temp, convert_size2string(location.end_line));

    // end_column
    buffer_append_str(temp, ", \"end_column\": ");
    buffer_append_str(temp, convert_size2string(location.end_column));

    // end_index
    buffer_append_str(temp, ", \"end_index\": ");
    buffer_append_str(temp, convert_size2string(location.end_index));

    // length
    buffer_append_str(temp, ", \"length\": ");
    buffer_append_str(temp, convert_size2string(location.length));
    buffer_append_char(temp, '}');

    char* result = string_duplicate(temp->data);
    buffer_destroy(temp);
    return result;
}
