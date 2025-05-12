#include <utility/string/string_convert_utf8_to_english_digit/string_convert_utf8_to_english_digit.h>

/**
 *
 * @function string_convert_utf8_to_english_digit
 * @brief Convert a UTF-8 string to an English digit
 * @params {char*} uc - UTF-8 string
 * @returns {char} English digit
 *
 */
char string_convert_utf8_to_english_digit(char *uc)
{
    DEBUG_ME;
    if (string_compare(uc, "۰") == 0 || string_compare(uc, "٠") == 0 || string_compare(uc, "0") == 0) {
        return '0';
    } else if (string_compare(uc, "۱") == 0 || string_compare(uc, "١") == 0 ||
               string_compare(uc, "1") == 0) {
        return '1';
    } else if (string_compare(uc, "۲") == 0 || string_compare(uc, "٢") == 0 ||
               string_compare(uc, "2") == 0) {
        return '2';
    } else if (string_compare(uc, "۳") == 0 || string_compare(uc, "٣") == 0 ||
               string_compare(uc, "3") == 0) {
        return '3';
    } else if (string_compare(uc, "۴") == 0 || string_compare(uc, "٤") == 0 ||
               string_compare(uc, "4") == 0) {
        return '4';
    } else if (string_compare(uc, "۵") == 0 || string_compare(uc, "٥") == 0 ||
               string_compare(uc, "5") == 0) {
        return '5';
    } else if (string_compare(uc, "۶") == 0 || string_compare(uc, "٦") == 0 ||
               string_compare(uc, "6") == 0) {
        return '6';
    } else if (string_compare(uc, "۷") == 0 || string_compare(uc, "٧") == 0 ||
               string_compare(uc, "7") == 0) {
        return '7';
    } else if (string_compare(uc, "۸") == 0 || string_compare(uc, "٨") == 0 ||
               string_compare(uc, "8") == 0) {
        return '8';
    } else if (string_compare(uc, "۹") == 0 || string_compare(uc, "٩") == 0 ||
               string_compare(uc, "9") == 0) {
        return '9';
    }

    return '\0';
}

