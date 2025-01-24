export enum TokenType {
    // Values
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_BOOLEAN,
    TOKEN_NUMBER_FLOAT,
    TOKEN_NUMBER_INT,
    TOKEN_BOOL_TRUE,
    TOKEN_BOOL_FALSE,

    // Words
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_PRINT,
    TOKEN_FN,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_REPEAT,
    TOKEN_DO,
    TOKEN_RET,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_LAYOUT,
    TOKEN_BLOCK_BEGIN,
    TOKEN_BLOCK_END,

    // Operators
    TOKEN_NOT_EQUAL,
    TOKEN_EQUAL,
    TOKEN_AND_AND,
    TOKEN_OR_OR,
    TOKEN_LESS_EQUAL,
    TOKEN_GREATER_EQUAL,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_SHIFT_LEFT,
    TOKEN_SHIFT_RIGHT,
    TOKEN_SHIFT_RIGHT_ASSIGN,
    TOKEN_SHIFT_LEFT_ASSIGN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MOD,
    TOKEN_POWER,
    TOKEN_ASSIGN,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_NOT,
    TOKEN_AND_BIT,
    TOKEN_OR_BIT,
    TOKEN_XOR_BIT,

    // Others
    TOKEN_ERROR,
    TOKEN_EOF,
};
