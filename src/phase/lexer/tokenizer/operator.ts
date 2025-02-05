import { TokenType } from './type';
import { TokenOperatorType } from './type';

export const operatorTypeMaps: Record<string, TokenOperatorType> = {
    "?": TokenOperatorType.TOKEN_QUESTION,
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
    "->": TokenOperatorType.TOKEN_MEMBER_POINTER,
    "−>": TokenOperatorType.TOKEN_MEMBER_POINTER,
    ".": TokenOperatorType.TOKEN_MEMBER,
    "..": TokenOperatorType.TOKEN_RANGE,
    "*": TokenOperatorType.TOKEN_MULTIPLY,
    "×": TokenOperatorType.TOKEN_MULTIPLY,
    "/": TokenOperatorType.TOKEN_DIVIDE,
    // "//": TokenOperatorType.TOKEN_DIVIDE_INT,
    "÷": TokenOperatorType.TOKEN_DIVIDE,
    // "÷÷": TokenOperatorType.TOKEN_DIVIDE_INT,
    "%": TokenOperatorType.TOKEN_MODULO,
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

export function isOperator(type: TokenType): boolean {
    return Object.values(TokenOperatorType).includes(type as TokenOperatorType);
}

export const invalidOperators: TokenType[] = [
    TokenOperatorType.TOKEN_RIGHT_PAREN,
    TokenOperatorType.TOKEN_RIGHT_BRACE,
    TokenOperatorType.TOKEN_RIGHT_BRACKET,
];
