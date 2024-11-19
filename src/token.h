/**
 * ---------------------------------------------------------------------------
 * Salam Programming Language
 * ---------------------------------------------------------------------------
 * 
 * Welcome to the Salam Programming Language! Salam is a modern, efficient, 
 * and developer-friendly language created to inspire and empower programmers 
 * of all ages. With its unique features and Persian roots, Salam aims to make 
 * programming more accessible and enjoyable for everyone.
 * 
 * This file is part of the core implementation of Salam, including its runtime 
 * and compiler components. By contributing to or using this codebase, you are 
 * part of a growing community dedicated to innovation and inclusivity in 
 * programming.
 * 
 * Explore Salam further:
 * - Website: https://www.salamlang.ir/
 * - GitHub: https://github.com/SalamLang/Salam
 * 
 * Thank you for being part of this journey!
 * ---------------------------------------------------------------------------
 */

// ADD_KEYWORD(TOKEN_TYPE, TOKEN_NAME, TOKEN_VALUE, TOKEN_VALUE_LENGTH)

#define TOKEN_BOOL_TRUE "true"
#define TOKEN_BOOL_FALSE "false"
#define TOKEN_NAME_UNKNOWN "UNKNOWN"
#define TOKEN_TYPE_OPEN_BLOCK TOKEN_COLON
#define TOKEN_TYPE_CLOSE_BLOCK TOKEN_BLOCK_CLOSE

#define TOKEN_BEGIN_KEYWORD ":"
#define TOKEN_END_KEYWORD "تمام"
#define TOKEN_ATTRIBUTE_KEYWORD "="
#define TOKEN_STRING_BEGIN_KEYWORD "«"
#define TOKEN_STRING_END_KEYWORD "»"

ADD_TOKEN(TOKEN_EOF, "EOF", "EOF")
ADD_TOKEN(TOKEN_ERROR, "ERROR", "ERROR")

ADD_KEYWORD(TOKEN_BLOCK_CLOSE, "BLOCK_CLOSE", TOKEN_END_KEYWORD, 4)
// ADD_KEYWORD_REPEAT(TOKEN_BLOCK_CLOSE, "BLOCK_CLOSE", "پایان", 4)

// ADD_KEYWORD(TOKEN_LAYOUT, "LAYOUT", "layout", 6)
ADD_KEYWORD(TOKEN_LAYOUT, "LAYOUT", "صفحه", 4)
// ADD_KEYWORD(TOKEN_IMPORT, "IMPORT", "import", 6)
ADD_KEYWORD(TOKEN_FUNCTION, "FUNCTION", "تابع", 4)
// ADD_KEYWORD(TOKEN_FUNCTION, "FUNCTION", "fn", 2)
ADD_KEYWORD(TOKEN_RETURN, "RETURN", "برگشت", 5)
// ADD_KEYWORD(TOKEN_RETURN, "RETURN", "ret", 3)
ADD_KEYWORD(TOKEN_IF, "IF", "اگر", 3)
// ADD_KEYWORD(TOKEN_IF, "IF", "if", 2)
// ADD_KEYWORD(TOKEN_PRINT, "PRINT", "print", 5)
ADD_KEYWORD(TOKEN_PRINT, "PRINT", "نمایش", 5)
ADD_KEYWORD(TOKEN_ELSE, "ELSE", "else", 4)
ADD_KEYWORD(TOKEN_WHILE, "WHILE", "while", 5)
ADD_KEYWORD(TOKEN_FOR, "FOR", "for", 3)
ADD_KEYWORD(TOKEN_BREAK, "BREAK", "break", 5)
ADD_KEYWORD(TOKEN_CONTINUE, "CONTINUE", "continue", 8)
ADD_KEYWORD(TOKEN_BOOLEAN, "BOOLEAN", "true", 4)
ADD_KEYWORD_HIDE(TOKEN_BOOLEAN, "BOOLEAN", "false", 5)

ADD_TOKEN(TOKEN_IDENTIFIER, "IDENTIFIER", "IDENTIFIER")
ADD_TOKEN(TOKEN_STRING, "STRING", "STRING")
ADD_TOKEN(TOKEN_NUMBER_INT, "NUMBER_INT", "NUMBER_INT")
ADD_TOKEN(TOKEN_NUMBER_FLOAT, "NUMBER_FLOAT", "NUMBER_FLOAT")
ADD_TOKEN(TOKEN_NOT_EQUAL, "NOT_EQUAL", "!=")
ADD_TOKEN(TOKEN_EQUAL, "EQUAL", "==")
ADD_TOKEN(TOKEN_AND_AND, "AND_AND", "&&")
ADD_TOKEN(TOKEN_OR_OR, "OR_OR", "||")
ADD_TOKEN(TOKEN_LESS_EQUAL, "LESS_EQUAL", "<=")
ADD_TOKEN(TOKEN_GREATER_EQUAL, "GREATER_EQUAL", ">=")
ADD_TOKEN(TOKEN_INCREMENT, "INCREMENT", "++")
ADD_TOKEN(TOKEN_DECREMENT, "DECREMENT", "--")
ADD_TOKEN(TOKEN_SHIFT_LEFT, "SHIFT_LEFT", "<<")
ADD_TOKEN(TOKEN_SHIFT_RIGHT, "SHIFT_RIGHT", ">>")
ADD_TOKEN(TOKEN_SHIFT_LEFT_ASSIGN, "SHIFT_LEFT_ASSIGN", "<<=")
ADD_TOKEN(TOKEN_SHIFT_RIGHT_ASSIGN, "SHIFT_RIGHT_ASSIGN", ">>=")

ADD_CHAR_TOKEN(TOKEN_LEFT_BRACE, "LEFT_BRACE", "{", '{')
ADD_CHAR_TOKEN(TOKEN_RIGHT_BRACE, "RIGHT_BRACE", "}", '}')

ADD_CHAR_TOKEN(TOKEN_LEFT_BRACKET, "LEFT_BRACKET", "[", '[')
ADD_CHAR_TOKEN(TOKEN_RIGHT_BRACKET, "RIGHT_BRACKET", "]", ']')
ADD_CHAR_TOKEN(TOKEN_COLON, "COLON", ":", ':')
ADD_CHAR_TOKEN(TOKEN_COMMA, "COMMA", ",", ',')
ADD_CHAR_TOKEN(TOKEN_LEFT_PAREN, "LEFT_PAREN", "(", '(')
ADD_CHAR_TOKEN(TOKEN_RIGHT_PAREN, "RIGHT_PAREN", ")", ')')
ADD_CHAR_TOKEN(TOKEN_PLUS, "PLUS", "+", '+')
ADD_CHAR_TOKEN(TOKEN_MINUS, "MINUS", "-", '-')
ADD_CHAR_TOKEN(TOKEN_MULTIPLY, "MULTIPLY", "*", '*')
ADD_CHAR_TOKEN(TOKEN_DIVIDE, "DIVIDE", "/", '/')
ADD_CHAR_TOKEN(TOKEN_MOD, "MOD", "%", '%')
ADD_CHAR_TOKEN(TOKEN_POWER, "POWER", "^", '^')
ADD_CHAR_TOKEN(TOKEN_ASSIGN, "ASSIGN", "=", '=')
ADD_CHAR_TOKEN(TOKEN_LESS, "LESS", "<", '<')
ADD_CHAR_TOKEN(TOKEN_GREATER, "GREATER", ">", '>')
ADD_CHAR_TOKEN(TOKEN_NOT, "NOT", "!", '!')
ADD_CHAR_TOKEN(TOKEN_AND_BIT, "AND_BIT", "&", '&')
ADD_CHAR_TOKEN(TOKEN_OR_BIT, "OR_BIT", "|", '|')
