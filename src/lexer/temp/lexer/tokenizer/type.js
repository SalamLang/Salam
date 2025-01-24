"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.operatorTypeMaps = exports.TokenType = void 0;
var TokenType;
(function (TokenType) {
    // Values
    TokenType[TokenType["TOKEN_STRING"] = 0] = "TOKEN_STRING";
    TokenType[TokenType["TOKEN_IDENTIFIER"] = 1] = "TOKEN_IDENTIFIER";
    TokenType[TokenType["TOKEN_BOOLEAN"] = 2] = "TOKEN_BOOLEAN";
    TokenType[TokenType["TOKEN_NUMBER_FLOAT"] = 3] = "TOKEN_NUMBER_FLOAT";
    TokenType[TokenType["TOKEN_NUMBER_INT"] = 4] = "TOKEN_NUMBER_INT";
    TokenType[TokenType["TOKEN_BOOL_TRUE"] = 5] = "TOKEN_BOOL_TRUE";
    TokenType[TokenType["TOKEN_BOOL_FALSE"] = 6] = "TOKEN_BOOL_FALSE";
    // Words
    TokenType[TokenType["TOKEN_IF"] = 7] = "TOKEN_IF";
    TokenType[TokenType["TOKEN_ELSE"] = 8] = "TOKEN_ELSE";
    TokenType[TokenType["TOKEN_PRINT"] = 9] = "TOKEN_PRINT";
    TokenType[TokenType["TOKEN_FN"] = 10] = "TOKEN_FN";
    TokenType[TokenType["TOKEN_FOR"] = 11] = "TOKEN_FOR";
    TokenType[TokenType["TOKEN_WHILE"] = 12] = "TOKEN_WHILE";
    TokenType[TokenType["TOKEN_REPEAT"] = 13] = "TOKEN_REPEAT";
    TokenType[TokenType["TOKEN_DO"] = 14] = "TOKEN_DO";
    TokenType[TokenType["TOKEN_RET"] = 15] = "TOKEN_RET";
    TokenType[TokenType["TOKEN_BREAK"] = 16] = "TOKEN_BREAK";
    TokenType[TokenType["TOKEN_CONTINUE"] = 17] = "TOKEN_CONTINUE";
    TokenType[TokenType["TOKEN_LAYOUT"] = 18] = "TOKEN_LAYOUT";
    TokenType[TokenType["TOKEN_BLOCK_BEGIN"] = 19] = "TOKEN_BLOCK_BEGIN";
    TokenType[TokenType["TOKEN_BLOCK_END"] = 20] = "TOKEN_BLOCK_END";
    // Operators
    TokenType[TokenType["TOKEN_NOT_EQUAL"] = 21] = "TOKEN_NOT_EQUAL";
    TokenType[TokenType["TOKEN_EQUAL"] = 22] = "TOKEN_EQUAL";
    TokenType[TokenType["TOKEN_AND_AND"] = 23] = "TOKEN_AND_AND";
    TokenType[TokenType["TOKEN_OR_OR"] = 24] = "TOKEN_OR_OR";
    TokenType[TokenType["TOKEN_LESS_EQUAL"] = 25] = "TOKEN_LESS_EQUAL";
    TokenType[TokenType["TOKEN_GREATER_EQUAL"] = 26] = "TOKEN_GREATER_EQUAL";
    TokenType[TokenType["TOKEN_INCREMENT"] = 27] = "TOKEN_INCREMENT";
    TokenType[TokenType["TOKEN_DECREMENT"] = 28] = "TOKEN_DECREMENT";
    TokenType[TokenType["TOKEN_SHIFT_LEFT"] = 29] = "TOKEN_SHIFT_LEFT";
    TokenType[TokenType["TOKEN_SHIFT_RIGHT"] = 30] = "TOKEN_SHIFT_RIGHT";
    TokenType[TokenType["TOKEN_SHIFT_RIGHT_ASSIGN"] = 31] = "TOKEN_SHIFT_RIGHT_ASSIGN";
    TokenType[TokenType["TOKEN_SHIFT_LEFT_ASSIGN"] = 32] = "TOKEN_SHIFT_LEFT_ASSIGN";
    TokenType[TokenType["TOKEN_LEFT_BRACE"] = 33] = "TOKEN_LEFT_BRACE";
    TokenType[TokenType["TOKEN_RIGHT_BRACE"] = 34] = "TOKEN_RIGHT_BRACE";
    TokenType[TokenType["TOKEN_LEFT_BRACKET"] = 35] = "TOKEN_LEFT_BRACKET";
    TokenType[TokenType["TOKEN_RIGHT_BRACKET"] = 36] = "TOKEN_RIGHT_BRACKET";
    TokenType[TokenType["TOKEN_COLON"] = 37] = "TOKEN_COLON";
    TokenType[TokenType["TOKEN_COLON_COLON"] = 38] = "TOKEN_COLON_COLON";
    TokenType[TokenType["TOKEN_COMMA"] = 39] = "TOKEN_COMMA";
    TokenType[TokenType["TOKEN_LEFT_PAREN"] = 40] = "TOKEN_LEFT_PAREN";
    TokenType[TokenType["TOKEN_RIGHT_PAREN"] = 41] = "TOKEN_RIGHT_PAREN";
    TokenType[TokenType["TOKEN_PLUS"] = 42] = "TOKEN_PLUS";
    TokenType[TokenType["TOKEN_MINUS"] = 43] = "TOKEN_MINUS";
    TokenType[TokenType["TOKEN_MULTIPLY"] = 44] = "TOKEN_MULTIPLY";
    TokenType[TokenType["TOKEN_DIVIDE"] = 45] = "TOKEN_DIVIDE";
    TokenType[TokenType["TOKEN_DIVIDE_INT"] = 46] = "TOKEN_DIVIDE_INT";
    TokenType[TokenType["TOKEN_MOD"] = 47] = "TOKEN_MOD";
    TokenType[TokenType["TOKEN_POWER"] = 48] = "TOKEN_POWER";
    TokenType[TokenType["TOKEN_ASSIGN"] = 49] = "TOKEN_ASSIGN";
    TokenType[TokenType["TOKEN_LESS"] = 50] = "TOKEN_LESS";
    TokenType[TokenType["TOKEN_GREATER"] = 51] = "TOKEN_GREATER";
    TokenType[TokenType["TOKEN_NOT"] = 52] = "TOKEN_NOT";
    TokenType[TokenType["TOKEN_AND_BIT"] = 53] = "TOKEN_AND_BIT";
    TokenType[TokenType["TOKEN_OR_BIT"] = 54] = "TOKEN_OR_BIT";
    TokenType[TokenType["TOKEN_XOR_BIT"] = 55] = "TOKEN_XOR_BIT";
    TokenType[TokenType["TOKEN_MEMBER"] = 56] = "TOKEN_MEMBER";
    TokenType[TokenType["TOKEN_RANGE"] = 57] = "TOKEN_RANGE";
    // Others
    TokenType[TokenType["TOKEN_ERROR"] = 58] = "TOKEN_ERROR";
    TokenType[TokenType["TOKEN_EOF"] = 59] = "TOKEN_EOF";
})(TokenType || (exports.TokenType = TokenType = {}));
;
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
