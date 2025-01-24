"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.operatorTypeMaps = exports.TokenTypeCombined = exports.TokenOperatorType = exports.TokenValueType = exports.TokenKeywordType = void 0;
var TokenKeywordType;
(function (TokenKeywordType) {
    TokenKeywordType[TokenKeywordType["TOKEN_IDENTIFIER"] = 0] = "TOKEN_IDENTIFIER";
    TokenKeywordType[TokenKeywordType["TOKEN_IF"] = 1] = "TOKEN_IF";
    TokenKeywordType[TokenKeywordType["TOKEN_ELSE"] = 2] = "TOKEN_ELSE";
    TokenKeywordType[TokenKeywordType["TOKEN_PRINT"] = 3] = "TOKEN_PRINT";
    TokenKeywordType[TokenKeywordType["TOKEN_FN"] = 4] = "TOKEN_FN";
    TokenKeywordType[TokenKeywordType["TOKEN_FOR"] = 5] = "TOKEN_FOR";
    TokenKeywordType[TokenKeywordType["TOKEN_WHILE"] = 6] = "TOKEN_WHILE";
    TokenKeywordType[TokenKeywordType["TOKEN_REPEAT"] = 7] = "TOKEN_REPEAT";
    TokenKeywordType[TokenKeywordType["TOKEN_DO"] = 8] = "TOKEN_DO";
    TokenKeywordType[TokenKeywordType["TOKEN_RET"] = 9] = "TOKEN_RET";
    TokenKeywordType[TokenKeywordType["TOKEN_BREAK"] = 10] = "TOKEN_BREAK";
    TokenKeywordType[TokenKeywordType["TOKEN_CONTINUE"] = 11] = "TOKEN_CONTINUE";
    TokenKeywordType[TokenKeywordType["TOKEN_LAYOUT"] = 12] = "TOKEN_LAYOUT";
    // TOKEN_BLOCK_BEGIN,
    TokenKeywordType[TokenKeywordType["TOKEN_BLOCK_END"] = 13] = "TOKEN_BLOCK_END";
})(TokenKeywordType || (exports.TokenKeywordType = TokenKeywordType = {}));
;
var TokenValueType;
(function (TokenValueType) {
    TokenValueType[TokenValueType["TOKEN_STRING"] = 0] = "TOKEN_STRING";
    TokenValueType[TokenValueType["TOKEN_BOOLEAN"] = 1] = "TOKEN_BOOLEAN";
    TokenValueType[TokenValueType["TOKEN_NUMBER_FLOAT"] = 2] = "TOKEN_NUMBER_FLOAT";
    TokenValueType[TokenValueType["TOKEN_NUMBER_INT"] = 3] = "TOKEN_NUMBER_INT";
    TokenValueType[TokenValueType["TOKEN_BOOL_TRUE"] = 4] = "TOKEN_BOOL_TRUE";
    TokenValueType[TokenValueType["TOKEN_BOOL_FALSE"] = 5] = "TOKEN_BOOL_FALSE";
})(TokenValueType || (exports.TokenValueType = TokenValueType = {}));
;
var TokenOperatorType;
(function (TokenOperatorType) {
    // TOKEN_BLOCK_BEGIN,
    // TOKEN_BLOCK_END,
    TokenOperatorType[TokenOperatorType["TOKEN_NOT_EQUAL"] = 0] = "TOKEN_NOT_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_EQUAL"] = 1] = "TOKEN_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_AND_AND"] = 2] = "TOKEN_AND_AND";
    TokenOperatorType[TokenOperatorType["TOKEN_OR_OR"] = 3] = "TOKEN_OR_OR";
    TokenOperatorType[TokenOperatorType["TOKEN_LESS_EQUAL"] = 4] = "TOKEN_LESS_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_GREATER_EQUAL"] = 5] = "TOKEN_GREATER_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_INCREMENT"] = 6] = "TOKEN_INCREMENT";
    TokenOperatorType[TokenOperatorType["TOKEN_DECREMENT"] = 7] = "TOKEN_DECREMENT";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_LEFT"] = 8] = "TOKEN_SHIFT_LEFT";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_RIGHT"] = 9] = "TOKEN_SHIFT_RIGHT";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_RIGHT_ASSIGN"] = 10] = "TOKEN_SHIFT_RIGHT_ASSIGN";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_LEFT_ASSIGN"] = 11] = "TOKEN_SHIFT_LEFT_ASSIGN";
    TokenOperatorType[TokenOperatorType["TOKEN_LEFT_BRACE"] = 12] = "TOKEN_LEFT_BRACE";
    TokenOperatorType[TokenOperatorType["TOKEN_RIGHT_BRACE"] = 13] = "TOKEN_RIGHT_BRACE";
    TokenOperatorType[TokenOperatorType["TOKEN_LEFT_BRACKET"] = 14] = "TOKEN_LEFT_BRACKET";
    TokenOperatorType[TokenOperatorType["TOKEN_RIGHT_BRACKET"] = 15] = "TOKEN_RIGHT_BRACKET";
    TokenOperatorType[TokenOperatorType["TOKEN_COLON"] = 16] = "TOKEN_COLON";
    TokenOperatorType[TokenOperatorType["TOKEN_COLON_COLON"] = 17] = "TOKEN_COLON_COLON";
    TokenOperatorType[TokenOperatorType["TOKEN_COMMA"] = 18] = "TOKEN_COMMA";
    TokenOperatorType[TokenOperatorType["TOKEN_LEFT_PAREN"] = 19] = "TOKEN_LEFT_PAREN";
    TokenOperatorType[TokenOperatorType["TOKEN_RIGHT_PAREN"] = 20] = "TOKEN_RIGHT_PAREN";
    TokenOperatorType[TokenOperatorType["TOKEN_PLUS"] = 21] = "TOKEN_PLUS";
    TokenOperatorType[TokenOperatorType["TOKEN_MINUS"] = 22] = "TOKEN_MINUS";
    TokenOperatorType[TokenOperatorType["TOKEN_MULTIPLY"] = 23] = "TOKEN_MULTIPLY";
    TokenOperatorType[TokenOperatorType["TOKEN_DIVIDE"] = 24] = "TOKEN_DIVIDE";
    TokenOperatorType[TokenOperatorType["TOKEN_DIVIDE_INT"] = 25] = "TOKEN_DIVIDE_INT";
    TokenOperatorType[TokenOperatorType["TOKEN_MOD"] = 26] = "TOKEN_MOD";
    TokenOperatorType[TokenOperatorType["TOKEN_POWER"] = 27] = "TOKEN_POWER";
    TokenOperatorType[TokenOperatorType["TOKEN_ASSIGN"] = 28] = "TOKEN_ASSIGN";
    TokenOperatorType[TokenOperatorType["TOKEN_LESS"] = 29] = "TOKEN_LESS";
    TokenOperatorType[TokenOperatorType["TOKEN_GREATER"] = 30] = "TOKEN_GREATER";
    TokenOperatorType[TokenOperatorType["TOKEN_NOT"] = 31] = "TOKEN_NOT";
    TokenOperatorType[TokenOperatorType["TOKEN_AND_BIT"] = 32] = "TOKEN_AND_BIT";
    TokenOperatorType[TokenOperatorType["TOKEN_OR_BIT"] = 33] = "TOKEN_OR_BIT";
    TokenOperatorType[TokenOperatorType["TOKEN_XOR_BIT"] = 34] = "TOKEN_XOR_BIT";
    TokenOperatorType[TokenOperatorType["TOKEN_MEMBER"] = 35] = "TOKEN_MEMBER";
    TokenOperatorType[TokenOperatorType["TOKEN_RANGE"] = 36] = "TOKEN_RANGE";
})(TokenOperatorType || (exports.TokenOperatorType = TokenOperatorType = {}));
;
var TokenType;
(function (TokenType) {
    const TOKEN_ERROR = 900;
    const TOKEN_EOF = 901;
    TokenType.Combined = {
        ...TokenKeywordType,
        ...TokenValueType,
        ...TokenOperatorType,
        TOKEN_ERROR: TOKEN_ERROR,
        TOKEN_EOF: TOKEN_EOF,
    };
    // export type CombinedType = typeof Combined[keyof typeof Combined];
})(TokenType || (TokenType = {}));
;
exports.TokenTypeCombined = TokenType.Combined;
// export type TokenType = keyof typeof TokenTypeCombined;
// export const TOKEN_BLOCK_BEGIN = [TokenKeywordType, TokenKeywordType.TOKEN_BLOCK_BEGIN];
// export const TOKEN_BLOCK_BEGIN = [TokenType, MergedEnum.TOKEN_COLON];
// export const TOKEN_BLOCK_END = [TokenKeywordType, TokenKeywordType.TOKEN_BLOCK_END];
exports.operatorTypeMaps = {
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
    "//": TokenOperatorType.TOKEN_DIVIDE_INT,
    "÷": TokenOperatorType.TOKEN_DIVIDE,
    "÷÷": TokenOperatorType.TOKEN_DIVIDE_INT,
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
