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
        [MESSAGE_PARSER_UNEXPECTED_TOKEN] = "Error: Unexpected token as statement %s\n",
        [MESSAGE_PARSER_BLOCK_MEMORY_ISSUE] = "Error: in parsing block: Memory reallocation failed.\n",
        [MESSAGE_PARSER_BAD_TOKEN_AS_STATEMENT] = "Error: bad token as statement %s\n",
        [MESSAGE_TOKEN_TRUE] = "غلط",
        [MESSAGE_TOKEN_FALSE] = "غلط",
        [MESSAGE_TOKEN_NULL] = "پوچ",
        [MESSAGE_TOKEN_UNKNOWN] = "نامشخص",

        [MESSAGE_TOKEN_OR] = "یا",
        [MESSAGE_TOKEN_AND] = "و",
        [MESSAGE_TOKEN_NUMBER_INT] = "عددصحیح",
        [MESSAGE_TOKEN_NUMBER_FLOAT] = "عدداعشار",
        [MESSAGE_TOKEN_BOOL] = "بولی",
        [MESSAGE_TOKEN_STRING] = "رشته",
        [MESSAGE_TOKEN_ARRAY] = "دسته",

        [MESSAGE_TOKEN_FUNCTION_TYPE] = "نوع",
        [MESSAGE_TOKEN_FUNCTION_EVEN] = "زوج",
        [MESSAGE_TOKEN_FUNCTION_ODD] = "فرد",
        [MESSAGE_TOKEN_FUNCTION_READ] = "خواندن",
        [MESSAGE_TOKEN_FUNCTION_LENGTH] = "طول",
        [MESSAGE_TOKEN_FUNCTION_STRING] = "رشته",

        [MESSAGE_INTERPRETER_FUNCTION_CALL_FIRST_ARGUMENT_SHOULD_BE_ONLY_A_STRING] = "Error: argument type of %S() function should be a string!\n",
        [MESSAGE_LEXER_FILE_NOT_EXISTS] = "Error: file %s not found\n",
        [MESSAGE_MEMORY_ALLOCATE_ERROR] = "Memory allocation error\n",
    },

    [LANGUAGE_ARABIC] = {
    }
};

const keyword_mapping_t keyword_mapping[2][50] = {
    [LANGUAGE_PERSIAN] = {
        {"تابع", TOKEN_TYPE_FUNCTION},
        {"برگشت", TOKEN_TYPE_RETURN},
        {"ادامه", TOKEN_TYPE_CONTINUE},
        {"توقف", TOKEN_TYPE_BREAK},
        {"نمایش", TOKEN_TYPE_PRINT},
        {"واگرنه", TOKEN_TYPE_ELSEIF},
        {"اگر", TOKEN_TYPE_IF},
        {"پوچ", TOKEN_TYPE_NULL},
        {"تا", TOKEN_TYPE_UNTIL},
        {"تکرار", TOKEN_TYPE_REPEAT},
        {"درست", TOKEN_TYPE_TRUE},
        {"غلط", TOKEN_TYPE_FALSE},
        {"و", TOKEN_TYPE_AND},
        {"یا", TOKEN_TYPE_OR},
        {NULL, TOKEN_TYPE_ERROR},
    },

    [LANGUAGE_ARABIC] = {
        {"تابع", TOKEN_TYPE_FUNCTION},
        {"برگشت", TOKEN_TYPE_RETURN},
        {"ادامه", TOKEN_TYPE_CONTINUE},
        {"توقف", TOKEN_TYPE_BREAK},
        {"نمایش", TOKEN_TYPE_PRINT},
        {"واگرنه", TOKEN_TYPE_ELSEIF},
        {"اگر", TOKEN_TYPE_IF},
        {"پوچ", TOKEN_TYPE_NULL},
        {"تا", TOKEN_TYPE_UNTIL},
        {"تکرار", TOKEN_TYPE_REPEAT},
        {"درست", TOKEN_TYPE_TRUE},
        {"غلط", TOKEN_TYPE_FALSE},
        {"و", TOKEN_TYPE_AND},
        {"یا", TOKEN_TYPE_OR},
        {NULL, TOKEN_TYPE_ERROR},
    },
};

#endif

