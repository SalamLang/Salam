export enum TokenKeywordType {
    TOKEN_IDENTIFIER = 100,
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
    // TOKEN_BLOCK_BEGIN,
    TOKEN_BLOCK_END,
};

export enum TokenValueType {
    TOKEN_STRING = 200,
    TOKEN_BOOLEAN,
    TOKEN_NUMBER_FLOAT,
    TOKEN_NUMBER_INT,
    TOKEN_BOOL_TRUE,
    TOKEN_BOOL_FALSE,
};

export enum TokenOperatorType {
    TOKEN_NOT_EQUAL = 300,
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
    TOKEN_COLON_COLON,
    TOKEN_COMMA,
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    // TOKEN_DIVIDE_INT,
    TOKEN_MOD,
    TOKEN_POWER,
    TOKEN_ASSIGN,
    TOKEN_LESS,
    TOKEN_GREATER,
    TOKEN_NOT,
    TOKEN_AND_BIT,
    TOKEN_OR_BIT,
    TOKEN_XOR_BIT,
    TOKEN_MEMBER,
    TOKEN_RANGE,
    // TOKEN_BLOCK_BEGIN,
    // TOKEN_BLOCK_END,
};

export enum TokenOtherType {
    TOKEN_ERROR = 400,
    TOKEN_EOF,
};

export const TokenTypeCombined = {
    ...TokenKeywordType,
    ...TokenValueType,
    ...TokenOperatorType,
    ...TokenOtherType,
} as const;

export function Token2Name(tokenType: TokenType): any {
    if (tokenType in TokenKeywordType) {
        return TokenKeywordType[tokenType];
    } else if (tokenType in TokenValueType) {
        return TokenValueType[tokenType];
    } else if (tokenType in TokenOperatorType) {
        return TokenOperatorType[tokenType];
    } else if (tokenType in TokenOtherType) {
        return TokenOtherType[tokenType];
    } else {
        return TokenOtherType.TOKEN_ERROR;
    }
};

export type TokenType = TokenKeywordType | TokenValueType | TokenOperatorType | TokenOtherType;

export const operatorTypeMaps: Record<string, TokenType> = {
    "!=": TokenOperatorType.TOKEN_NOT_EQUAL,
    "==": TokenOperatorType.TOKEN_EQUAL,
    "===": TokenOperatorType.TOKEN_EQUAL,
    "&&": TokenOperatorType.TOKEN_AND_AND,
    "||": TokenOperatorType.TOKEN_OR_OR,
    "<=": TokenOperatorType.TOKEN_LESS_EQUAL,
    ">=": TokenOperatorType.TOKEN_GREATER_EQUAL,
    "++": TokenOperatorType.TOKEN_INCREMENT,
    "＋＋": TokenOperatorType.TOKEN_INCREMENT,
    "﹢﹢": TokenOperatorType.TOKEN_INCREMENT,
    "--": TokenOperatorType.TOKEN_DECREMENT,
    "−−": TokenOperatorType.TOKEN_DECREMENT,
    "<<": TokenOperatorType.TOKEN_SHIFT_LEFT,
    ">>": TokenOperatorType.TOKEN_SHIFT_RIGHT,
    ">>=": TokenOperatorType.TOKEN_SHIFT_RIGHT_ASSIGN,
    "<<=": TokenOperatorType.TOKEN_SHIFT_LEFT_ASSIGN,
    "{": TokenOperatorType.TOKEN_LEFT_BRACE,
    "}": TokenOperatorType.TOKEN_RIGHT_BRACE,
    "[": TokenOperatorType.TOKEN_LEFT_BRACKET,
    "]": TokenOperatorType.TOKEN_RIGHT_BRACKET,
    ":": TokenOperatorType.TOKEN_COLON,
    "::": TokenOperatorType.TOKEN_COLON_COLON,
    ",": TokenOperatorType.TOKEN_COMMA,
    "(": TokenOperatorType.TOKEN_LEFT_PAREN,
    ")": TokenOperatorType.TOKEN_RIGHT_PAREN,
    "+": TokenOperatorType.TOKEN_PLUS,
    "﹢": TokenOperatorType.TOKEN_PLUS,
    "＋": TokenOperatorType.TOKEN_PLUS,
    "-": TokenOperatorType.TOKEN_MINUS,
    "−": TokenOperatorType.TOKEN_MINUS,
    "->": TokenOperatorType.TOKEN_MEMBER,
    ".": TokenOperatorType.TOKEN_MEMBER,
    "..": TokenOperatorType.TOKEN_RANGE,
    "*": TokenOperatorType.TOKEN_MULTIPLY,
    "×": TokenOperatorType.TOKEN_MULTIPLY,
    "/": TokenOperatorType.TOKEN_DIVIDE,
    // "//": TokenOperatorType.TOKEN_DIVIDE_INT,
    "÷": TokenOperatorType.TOKEN_DIVIDE,
    // "÷÷": TokenOperatorType.TOKEN_DIVIDE_INT,
    "%": TokenOperatorType.TOKEN_MOD,
    "**": TokenOperatorType.TOKEN_POWER,
    "××": TokenOperatorType.TOKEN_POWER,
    "=": TokenOperatorType.TOKEN_ASSIGN,
    "⩵": TokenOperatorType.TOKEN_EQUAL,
    "<": TokenOperatorType.TOKEN_LESS,
    ">": TokenOperatorType.TOKEN_GREATER,
    "!": TokenOperatorType.TOKEN_NOT,
    "&": TokenOperatorType.TOKEN_AND_BIT,
    "|": TokenOperatorType.TOKEN_OR_BIT,
    "^": TokenOperatorType.TOKEN_XOR_BIT,
    "~": TokenOperatorType.TOKEN_XOR_BIT,
};
