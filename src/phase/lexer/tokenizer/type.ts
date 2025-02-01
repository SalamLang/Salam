import { Token } from "./token";

export enum TokenKeywordDataType {
    TOKEN_DATA_TYPE_BOOL = "DATA_TYPE_BOOL",
    TOKEN_DATA_TYPE_INT = "DATA_TYPE_INT",
    TOKEN_DATA_TYPE_FLOAT= "DATA_TYPE_FLOAT",
    TOKEN_DATA_TYPE_STRING = "DATA_TYPE_STRING",
    TOKEN_DATA_TYPE_CHAR = "DATA_TYPE_CHAR",
};

export const TokenKeywordDataTypeValues: TokenKeywordDataType[] = Object.values(TokenKeywordDataType);

export function isDataTypeToken(token: Token): boolean {
    return TokenKeywordDataTypeValues.includes(token.type as TokenKeywordDataType);
};

export enum TokenKeywordType {
    TOKEN_IDENTIFIER = "IDENTIFIER",
    TOKEN_IF = "IF",
    TOKEN_ELSE = "ELSE",
    TOKEN_PRINT = "PRINT",
    TOKEN_FN = "FN",
    TOKEN_FOR = "FOR",
    TOKEN_WHILE = "WHILE",
    TOKEN_REPEAT = "REPEAT",
    TOKEN_DO = "DO",
    TOKEN_RET = "RET",
    TOKEN_BREAK = "BREAK",
    TOKEN_CONTINUE = "CONTINUE",
    TOKEN_LAYOUT = "LAYOUT",
    // TOKEN_BLOCK_BEGIN = "BLOCK_BEGIN",
    TOKEN_BLOCK_END = "BLOCK_END",
    TOKEN_BOOL_TRUE = "BOOL_TRUE",
    TOKEN_BOOL_FALSE = "BOOL_FALSE",
};

export enum TokenValueType {
    TOKEN_STRING = "STRING",
    TOKEN_NUMBER_FLOAT = "NUMBER_FLOAT",
    TOKEN_NUMBER_INT = "NUMBER_INT",
    TOKEN_BOOL_TRUE = "BOOL_TRUE",
    TOKEN_BOOL_FALSE = "BOOL_FALSE",
};

export enum TokenOperatorType {
    TOKEN_NOT_EQUAL = "NOT_EQUAL",
    TOKEN_EQUAL = "EQUAL",
    TOKEN_AND_AND = "AND_AND",
    TOKEN_OR_OR = "OR_OR",
    TOKEN_LESS_EQUAL = "LESS_EQUAL",
    TOKEN_GREATER_EQUAL = "GREATER_EQUAL",
    TOKEN_INCREMENT = "INCREMENT",
    TOKEN_DECREMENT = "DECREMENT",
    TOKEN_SHIFT_LEFT = "SHIFT_LEFT",
    TOKEN_SHIFT_RIGHT = "SHIFT_RIGHT",
    TOKEN_SHIFT_RIGHT_ASSIGN = "SHIFT_RIGHT_ASSIGN",
    TOKEN_SHIFT_LEFT_ASSIGN = "SHIFT_LEFT_ASSIGN",
    TOKEN_LEFT_BRACE = "LEFT_BRACE",
    TOKEN_RIGHT_BRACE = "RIGHT_BRACE",
    TOKEN_LEFT_BRACKET = "LEFT_BRACKET",
    TOKEN_RIGHT_BRACKET = "RIGHT_BRACKET",
    TOKEN_COLON = "COLON",
    TOKEN_COLON_COLON = "COLON_COLON",
    TOKEN_COMMA = "COMMA",
    TOKEN_LEFT_PAREN = "LEFT_PAREN",
    TOKEN_RIGHT_PAREN = "RIGHT_PAREN",
    TOKEN_PLUS = "PLUS",
    TOKEN_MINUS = "MINUS",
    TOKEN_MULTIPLY = "MULTIPLY",
    TOKEN_DIVIDE = "DIVIDE",
    // TOKEN_DIVIDE_INT = "DIVIDE_INT",
    TOKEN_MODULO = "MOD",
    TOKEN_POWER = "POWER",
    TOKEN_ASSIGN = "ASSIGN",
    TOKEN_LESS = "LESS",
    TOKEN_GREATER = "GREATER",
    TOKEN_NOT = "NOT",
    TOKEN_AND_BIT = "AND_BIT",
    TOKEN_OR_BIT = "OR_BIT",
    TOKEN_XOR_BIT = "XOR_BIT",
    TOKEN_MEMBER = "MEMBER",
    TOKEN_RANGE = "RANGE",
    // TOKEN_BLOCK_BEGIN = "BLOCK_BEGIN",
    // TOKEN_BLOCK_END = "BLOCK_END",
};

export enum TokenOtherType {
    TOKEN_ERROR = "ERROR",
    TOKEN_EOF = "EOF",
};

export type TokenType = TokenKeywordType | TokenKeywordDataType | TokenValueType | TokenOperatorType | TokenOtherType;
