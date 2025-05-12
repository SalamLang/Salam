/**
 *
 * @function utf8_is_digit
 * @brief Check if a UTF-8 string is an Persian/Arabic/English digit
 * @params {char*} utf8 - UTF-8 string
 * @returns {bool} True if the string is a Persian/Arabic/English digit,
 * false otherwise
 *
 */
bool utf8_is_digit(char *utf8)
{
    DEBUG_ME;
    // Persian digits
    if (strcmp(utf8, "۰") == 0 ||  // Persian 0
        strcmp(utf8, "۱") == 0 ||  // Persian 1
        strcmp(utf8, "۲") == 0 ||  // Persian 2
        strcmp(utf8, "۳") == 0 ||  // Persian 3
        strcmp(utf8, "۴") == 0 ||  // Persian 4
        strcmp(utf8, "۵") == 0 ||  // Persian 5
        strcmp(utf8, "۶") == 0 ||  // Persian 6
        strcmp(utf8, "۷") == 0 ||  // Persian 7
        strcmp(utf8, "۸") == 0 ||  // Persian 8
        strcmp(utf8, "۹") == 0     // Persian 9
    ) {
        return true;
    }
    // Arabic digits
    else if (strcmp(utf8, "٠") == 0 ||  // Arabic 0
             strcmp(utf8, "١") == 0 ||  // Arabic 1
             strcmp(utf8, "٢") == 0 ||  // Arabic 2
             strcmp(utf8, "٣") == 0 ||  // Arabic 3
             strcmp(utf8, "٤") == 0 ||  // Arabic 4
             strcmp(utf8, "٥") == 0 ||  // Arabic 5
             strcmp(utf8, "٦") == 0 ||  // Arabic 6
             strcmp(utf8, "٧") == 0 ||  // Arabic 7
             strcmp(utf8, "٨") == 0 ||  // Arabic 8
             strcmp(utf8, "٩") == 0     // Arabic 9
    ) {
        return true;
    }
    // English digits
    else if (strcmp(utf8, "0") == 0 ||  // English 0
             strcmp(utf8, "1") == 0 ||  // English 1
             strcmp(utf8, "2") == 0 ||  // English 2
             strcmp(utf8, "3") == 0 ||  // English 3
             strcmp(utf8, "4") == 0 ||  // English 4
             strcmp(utf8, "5") == 0 ||  // English 5
             strcmp(utf8, "6") == 0 ||  // English 6
             strcmp(utf8, "7") == 0 ||  // English 7
             strcmp(utf8, "8") == 0 ||  // English 8
             strcmp(utf8, "9") == 0     // English 9
    ) {
        return true;
    }

    return false;
}
