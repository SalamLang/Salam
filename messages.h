#ifdef _SALAM_LANGUAGE_

const char* messages[LANGUAGE_COUNT][MESSAGE_COUNT] = {
    [LANGUAGE_PERSIAN] = {
        [MESSAGE_NAME] = "سلام",
        [MESSAGE_WELCOME] = "Welcome to Salam Programming Language!\n"
            "Salam is the first Persian/Arabic Iranian computer scripting language.\n\n"

            "Usage:\n"
            "  salam <filename>\t\t\t# Execute a Salam script\n\n"
            "Example:\n"
            "  salam my_script.salam\t\t# Run the Salam script 'my_script.salam'\n\n"

            "Feel free to explore and create using Salam!\n"
            "For more information, visit: https://salamlang.ir\n\n",
		[MESSAGE_LEXER_UNEXPECTED_CHAR] = "Error: Unexpected character '%c' at line %zu, column %zu\n",
        [MESSAGE_LEXER_COMMENT_MULTI_NOT_CLOSED] = "Error: you have to close your multiline comments and it's not allowed to ignore and leave your multiline comment non-closed!\n",
        [MESSAGE_LEXER_NUMBER_FLOAT_NEED_NUMBER_AFTER_DOT] = "Syntax Error: we expect a number after dot of a float value\n",
        [MESSAGE_LEXER_TOKEN_UNREAD_UNICODE] = "Syntax Error: nuread - invalid unicode character\n",
        [MESSAGE_LEXER_TOKEN_READ_UNICODE] = "Syntax Error: read token - invalid unicode character\n",
        [MESSAGE_LEXER_STRING_READ_MEMORY] = "Error: read_string iterate - Memory reallocation failed.\n",
        [MESSAGE_LEXER_STRING_UNKNOWN_ESCAPE] = "Error: read_string - Unknown escape sequence\n",
        [MESSAGE_LEXER_STRING_CONVERT_MULTIBYTE] = "Error: read_string - Failed to convert wide character to multibyte\n",
    },

    [LANGUAGE_ARABIC] = {
    }
};

#endif
