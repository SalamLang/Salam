/**
 *
 * @function convert_float2string
 * @brief Convert a float to a string
 * @params {float} value - Float value
 * @returns {char*}
 *
 */
char *convert_float2string(float value)
{
    DEBUG_ME;
    static char buffer[256];
    snprintf(buffer, sizeof(buffer), "%f", value);

    return buffer;
}
