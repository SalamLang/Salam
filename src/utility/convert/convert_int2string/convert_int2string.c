/**
 *
 * @function convert_int2string
 * @brief Convert an integer to a string
 * @params {int} value - Integer value
 * @returns {char*}
 *
 */
char *convert_int2string(int value)
{
    DEBUG_ME;
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%d", value);

    return buffer;
}
