#ifdef _SALAM_LANGUAGE_

const char* messages[LANGUAGE_COUNT][MESSAGE_COUNT] = {
    [LANGUAGE_PERSIAN] = {
        [MESSAGE_NAME] = "سلام",
        [MESSAGE_ENTRY_POINT_FUNCTION_NAME] = "سلام",
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
        [MESSAGE_LEXER_IDENTIFIER_CONVERT_MULTIBYTE] = "Error: read identifier - Failed to convert wide character to multibyte\n",
        [MESSAGE_LEXER_STRING_GET_LENGTH_UNICODE] = "Error: in mbstowcs - count length\n",
        [MESSAGE_LEXER_CHAR_LENGTH_ISSUE] = "Error: in wcrtomb in wide character\n",
        [MESSAGE_LEXER_ARRAY_NOT_CLOSED] = "Error: Expected ',' or ']' in array value\n",
        [MESSAGE_INTERPRETER_MAIN_NORETURN] = "No return value from main function, so default!\n",
        [MESSAGE_INTERPRETER_CANNOT_HAVE_RET_BREAK_CON_OUT_OF_LOOP] = "Error: it's not possible to have break/continue inside a non-loop!\n",
        [MESSAGE_INTERPRETER_VARIABLE_NOT_FOUND] = "Error: Variable not found: %s\n",
        [MESSAGE_INTERPRETER_EXPRESSION_INVALID_VALUE_IN_BINARY] = "Error: cannot calculate binary operator for invalid values!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_COMPARE_THIS_KIND_OF_VALUE_TYPES] = "Error: cannot compare unknown types!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_AND_FOR_THIS_VALUES] = "Error: cannot calculate this values for AND operator!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_OR_FOR_THIS_VALUES] = "Error: cannot calculate this values for OR operator!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_BINARY_OP_FOR_NON_INT] = "Error: cannot calculate binary operator for non-int values!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_MODULE_OP_FOR_FLOAT] = "Error: cannot calculate %% operator for float values!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_DIVIDE_BY_ZERO] = "Error: cannot divide by zero!\n",
        [MESSAGE_INTERPRETER_EXPRESSION_CANNOT_DO_THIS_OPERATOR] = "Error: unknown operator: %s\n",
        [MESSAGE_INTERPRETER_FUNCTION_CALL_NUMBER_ARGS_IS_MORE] = "Error: number of arguments is not match with the function - you are passing more arguments!\n",
        [MESSAGE_INTERPRETER_FUNCTION_CALL_NUMBER_ARGS_IS_LESS] = "Error: number of arguments is not match with the function - you are passing less arguments!\n",
		[MESSAGE_INTERPRETER_FUNCTION_CALL_NUMBER_ARGS_SHOULD_BE_ONLY_ZERO] = "Error: number of arguments of %s() function should be only zero - this function don't accept any arguments!\n",
		[MESSAGE_INTERPRETER_FUNCTION_CALL_NUMBER_ARGS_SHOULD_BE_ONLY_ONE] = "Error: number of arguments of %s() function should be only one!\n",
        [MESSAGE_INTERPRETER_FUNCTION_NOT_EXISTS] = "Error: function not exists!\n",
        [MESSAGE_INTERPRETER_CANNOT_ASSIGN_VARIABLE_WITH_A_NON_IDENTIFIER_AS_NAME] = "Error: Assignment to non-variable\n",
        [MESSAGE_INTERPRETER_EXPRESSION_DONT_SUPPORT_THIS_TYPE_IN_EXPRESSION] = "Error: default expr type: %d\n",
    },

    [LANGUAGE_ARABIC] = {
    }
};

#endif
