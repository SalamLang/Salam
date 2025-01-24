"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.operatorTypeMaps = exports.TOKEN_BLOCK_BEGIN = exports.TokenType = exports.TokenKeywordType = void 0;
var TokenKeywordType;
(function (TokenKeywordType) {
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_IDENTIFIER"] = 0] = "TOKEN_KEYWORD_IDENTIFIER";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_IF"] = 1] = "TOKEN_KEYWORD_IF";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_ELSE"] = 2] = "TOKEN_KEYWORD_ELSE";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_PRINT"] = 3] = "TOKEN_KEYWORD_PRINT";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_FN"] = 4] = "TOKEN_KEYWORD_FN";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_FOR"] = 5] = "TOKEN_KEYWORD_FOR";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_WHILE"] = 6] = "TOKEN_KEYWORD_WHILE";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_REPEAT"] = 7] = "TOKEN_KEYWORD_REPEAT";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_DO"] = 8] = "TOKEN_KEYWORD_DO";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_RET"] = 9] = "TOKEN_KEYWORD_RET";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_BREAK"] = 10] = "TOKEN_KEYWORD_BREAK";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_CONTINUE"] = 11] = "TOKEN_KEYWORD_CONTINUE";
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_LAYOUT"] = 12] = "TOKEN_KEYWORD_LAYOUT";
    // TOKEN_KEYWORD_BEGIN,
    TokenKeywordType[TokenKeywordType["TOKEN_KEYWORD_BLOCK_END"] = 13] = "TOKEN_KEYWORD_BLOCK_END";
})(TokenKeywordType || (exports.TokenKeywordType = TokenKeywordType = {}));
;
var TokenType;
(function (TokenType) {
    // Values
    TokenType[TokenType["TOKEN_KEYWORD"] = 0] = "TOKEN_KEYWORD";
    TokenType[TokenType["TOKEN_STRING"] = 1] = "TOKEN_STRING";
    TokenType[TokenType["TOKEN_BOOLEAN"] = 2] = "TOKEN_BOOLEAN";
    TokenType[TokenType["TOKEN_NUMBER_FLOAT"] = 3] = "TOKEN_NUMBER_FLOAT";
    TokenType[TokenType["TOKEN_NUMBER_INT"] = 4] = "TOKEN_NUMBER_INT";
    TokenType[TokenType["TOKEN_BOOL_TRUE"] = 5] = "TOKEN_BOOL_TRUE";
    TokenType[TokenType["TOKEN_BOOL_FALSE"] = 6] = "TOKEN_BOOL_FALSE";
    // Operators
    TokenType[TokenType["TOKEN_BLOCK_BEGIN"] = 7] = "TOKEN_BLOCK_BEGIN";
    TokenType[TokenType["TOKEN_NOT_EQUAL"] = 8] = "TOKEN_NOT_EQUAL";
    TokenType[TokenType["TOKEN_EQUAL"] = 9] = "TOKEN_EQUAL";
    TokenType[TokenType["TOKEN_AND_AND"] = 10] = "TOKEN_AND_AND";
    TokenType[TokenType["TOKEN_OR_OR"] = 11] = "TOKEN_OR_OR";
    TokenType[TokenType["TOKEN_LESS_EQUAL"] = 12] = "TOKEN_LESS_EQUAL";
    TokenType[TokenType["TOKEN_GREATER_EQUAL"] = 13] = "TOKEN_GREATER_EQUAL";
    TokenType[TokenType["TOKEN_INCREMENT"] = 14] = "TOKEN_INCREMENT";
    TokenType[TokenType["TOKEN_DECREMENT"] = 15] = "TOKEN_DECREMENT";
    TokenType[TokenType["TOKEN_SHIFT_LEFT"] = 16] = "TOKEN_SHIFT_LEFT";
    TokenType[TokenType["TOKEN_SHIFT_RIGHT"] = 17] = "TOKEN_SHIFT_RIGHT";
    TokenType[TokenType["TOKEN_SHIFT_RIGHT_ASSIGN"] = 18] = "TOKEN_SHIFT_RIGHT_ASSIGN";
    TokenType[TokenType["TOKEN_SHIFT_LEFT_ASSIGN"] = 19] = "TOKEN_SHIFT_LEFT_ASSIGN";
    TokenType[TokenType["TOKEN_LEFT_BRACE"] = 20] = "TOKEN_LEFT_BRACE";
    TokenType[TokenType["TOKEN_RIGHT_BRACE"] = 21] = "TOKEN_RIGHT_BRACE";
    TokenType[TokenType["TOKEN_LEFT_BRACKET"] = 22] = "TOKEN_LEFT_BRACKET";
    TokenType[TokenType["TOKEN_RIGHT_BRACKET"] = 23] = "TOKEN_RIGHT_BRACKET";
    TokenType[TokenType["TOKEN_COLON"] = 24] = "TOKEN_COLON";
    TokenType[TokenType["TOKEN_COLON_COLON"] = 25] = "TOKEN_COLON_COLON";
    TokenType[TokenType["TOKEN_COMMA"] = 26] = "TOKEN_COMMA";
    TokenType[TokenType["TOKEN_LEFT_PAREN"] = 27] = "TOKEN_LEFT_PAREN";
    TokenType[TokenType["TOKEN_RIGHT_PAREN"] = 28] = "TOKEN_RIGHT_PAREN";
    TokenType[TokenType["TOKEN_PLUS"] = 29] = "TOKEN_PLUS";
    TokenType[TokenType["TOKEN_MINUS"] = 30] = "TOKEN_MINUS";
    TokenType[TokenType["TOKEN_MULTIPLY"] = 31] = "TOKEN_MULTIPLY";
    TokenType[TokenType["TOKEN_DIVIDE"] = 32] = "TOKEN_DIVIDE";
    TokenType[TokenType["TOKEN_DIVIDE_INT"] = 33] = "TOKEN_DIVIDE_INT";
    TokenType[TokenType["TOKEN_MOD"] = 34] = "TOKEN_MOD";
    TokenType[TokenType["TOKEN_POWER"] = 35] = "TOKEN_POWER";
    TokenType[TokenType["TOKEN_ASSIGN"] = 36] = "TOKEN_ASSIGN";
    TokenType[TokenType["TOKEN_LESS"] = 37] = "TOKEN_LESS";
    TokenType[TokenType["TOKEN_GREATER"] = 38] = "TOKEN_GREATER";
    TokenType[TokenType["TOKEN_NOT"] = 39] = "TOKEN_NOT";
    TokenType[TokenType["TOKEN_AND_BIT"] = 40] = "TOKEN_AND_BIT";
    TokenType[TokenType["TOKEN_OR_BIT"] = 41] = "TOKEN_OR_BIT";
    TokenType[TokenType["TOKEN_XOR_BIT"] = 42] = "TOKEN_XOR_BIT";
    TokenType[TokenType["TOKEN_MEMBER"] = 43] = "TOKEN_MEMBER";
    TokenType[TokenType["TOKEN_RANGE"] = 44] = "TOKEN_RANGE";
    // Others
    TokenType[TokenType["TOKEN_ERROR"] = 45] = "TOKEN_ERROR";
    TokenType[TokenType["TOKEN_EOF"] = 46] = "TOKEN_EOF";
})(TokenType || (exports.TokenType = TokenType = {}));
;
// export const TOKEN_BLOCK_BEGIN = TokenType.TOKEN_BEGIN;
exports.TOKEN_BLOCK_BEGIN = TokenType.TOKEN_COLON;
exports.operatorTypeMaps = {
    "!=": TokenType.TOKEN_NOT_EQUAL,
    "==": TokenType.TOKEN_EQUAL,
    "===": TokenType.TOKEN_EQUAL,
    "&&": TokenType.TOKEN_AND_AND,
    "||": TokenType.TOKEN_OR_OR,
    "<=": TokenType.TOKEN_LESS_EQUAL,
    ">=": TokenType.TOKEN_GREATER_EQUAL,
    "++": TokenType.TOKEN_INCREMENT,
    "＋＋": TokenType.TOKEN_INCREMENT,
    "﹢﹢": TokenType.TOKEN_INCREMENT,
    "--": TokenType.TOKEN_DECREMENT,
    "−−": TokenType.TOKEN_DECREMENT,
    "<<": TokenType.TOKEN_SHIFT_LEFT,
    ">>": TokenType.TOKEN_SHIFT_RIGHT,
    ">>=": TokenType.TOKEN_SHIFT_RIGHT_ASSIGN,
    "<<=": TokenType.TOKEN_SHIFT_LEFT_ASSIGN,
    "{": TokenType.TOKEN_LEFT_BRACE,
    "}": TokenType.TOKEN_RIGHT_BRACE,
    "[": TokenType.TOKEN_LEFT_BRACKET,
    "]": TokenType.TOKEN_RIGHT_BRACKET,
    ":": TokenType.TOKEN_COLON,
    "::": TokenType.TOKEN_COLON_COLON,
    ",": TokenType.TOKEN_COMMA,
    "(": TokenType.TOKEN_LEFT_PAREN,
    ")": TokenType.TOKEN_RIGHT_PAREN,
    "+": TokenType.TOKEN_PLUS,
    "﹢": TokenType.TOKEN_PLUS,
    "＋": TokenType.TOKEN_PLUS,
    "-": TokenType.TOKEN_MINUS,
    "−": TokenType.TOKEN_MINUS,
    "->": TokenType.TOKEN_MEMBER,
    ".": TokenType.TOKEN_MEMBER,
    "..": TokenType.TOKEN_RANGE,
    "*": TokenType.TOKEN_MULTIPLY,
    "×": TokenType.TOKEN_MULTIPLY,
    "/": TokenType.TOKEN_DIVIDE,
    "//": TokenType.TOKEN_DIVIDE_INT,
    "÷": TokenType.TOKEN_DIVIDE,
    "÷÷": TokenType.TOKEN_DIVIDE_INT,
    "%": TokenType.TOKEN_MOD,
    "**": TokenType.TOKEN_POWER,
    "××": TokenType.TOKEN_POWER,
    "=": TokenType.TOKEN_ASSIGN,
    "⩵": TokenType.TOKEN_EQUAL,
    "<": TokenType.TOKEN_LESS,
    ">": TokenType.TOKEN_GREATER,
    "!": TokenType.TOKEN_NOT,
    "&": TokenType.TOKEN_AND_BIT,
    "|": TokenType.TOKEN_OR_BIT,
    "^": TokenType.TOKEN_XOR_BIT,
    "~": TokenType.TOKEN_XOR_BIT,
};
