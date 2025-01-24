"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.operatorTypeMaps = exports.TokenOtherType = exports.TokenOperatorType = exports.TokenValueType = exports.TokenKeywordType = void 0;
var TokenKeywordType;
(function (TokenKeywordType) {
    TokenKeywordType[TokenKeywordType["TOKEN_IDENTIFIER"] = 100] = "TOKEN_IDENTIFIER";
    TokenKeywordType[TokenKeywordType["TOKEN_IF"] = 101] = "TOKEN_IF";
    TokenKeywordType[TokenKeywordType["TOKEN_ELSE"] = 102] = "TOKEN_ELSE";
    TokenKeywordType[TokenKeywordType["TOKEN_PRINT"] = 103] = "TOKEN_PRINT";
    TokenKeywordType[TokenKeywordType["TOKEN_FN"] = 104] = "TOKEN_FN";
    TokenKeywordType[TokenKeywordType["TOKEN_FOR"] = 105] = "TOKEN_FOR";
    TokenKeywordType[TokenKeywordType["TOKEN_WHILE"] = 106] = "TOKEN_WHILE";
    TokenKeywordType[TokenKeywordType["TOKEN_REPEAT"] = 107] = "TOKEN_REPEAT";
    TokenKeywordType[TokenKeywordType["TOKEN_DO"] = 108] = "TOKEN_DO";
    TokenKeywordType[TokenKeywordType["TOKEN_RET"] = 109] = "TOKEN_RET";
    TokenKeywordType[TokenKeywordType["TOKEN_BREAK"] = 110] = "TOKEN_BREAK";
    TokenKeywordType[TokenKeywordType["TOKEN_CONTINUE"] = 111] = "TOKEN_CONTINUE";
    TokenKeywordType[TokenKeywordType["TOKEN_LAYOUT"] = 112] = "TOKEN_LAYOUT";
    // TOKEN_BLOCK_BEGIN,
    TokenKeywordType[TokenKeywordType["TOKEN_BLOCK_END"] = 113] = "TOKEN_BLOCK_END";
})(TokenKeywordType || (exports.TokenKeywordType = TokenKeywordType = {}));
;
var TokenValueType;
(function (TokenValueType) {
    TokenValueType[TokenValueType["TOKEN_STRING"] = 200] = "TOKEN_STRING";
    TokenValueType[TokenValueType["TOKEN_BOOLEAN"] = 201] = "TOKEN_BOOLEAN";
    TokenValueType[TokenValueType["TOKEN_NUMBER_FLOAT"] = 202] = "TOKEN_NUMBER_FLOAT";
    TokenValueType[TokenValueType["TOKEN_NUMBER_INT"] = 203] = "TOKEN_NUMBER_INT";
    TokenValueType[TokenValueType["TOKEN_BOOL_TRUE"] = 204] = "TOKEN_BOOL_TRUE";
    TokenValueType[TokenValueType["TOKEN_BOOL_FALSE"] = 205] = "TOKEN_BOOL_FALSE";
})(TokenValueType || (exports.TokenValueType = TokenValueType = {}));
;
var TokenOperatorType;
(function (TokenOperatorType) {
    // TOKEN_BLOCK_BEGIN,
    // TOKEN_BLOCK_END,
    TokenOperatorType[TokenOperatorType["TOKEN_NOT_EQUAL"] = 300] = "TOKEN_NOT_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_EQUAL"] = 301] = "TOKEN_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_AND_AND"] = 302] = "TOKEN_AND_AND";
    TokenOperatorType[TokenOperatorType["TOKEN_OR_OR"] = 303] = "TOKEN_OR_OR";
    TokenOperatorType[TokenOperatorType["TOKEN_LESS_EQUAL"] = 304] = "TOKEN_LESS_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_GREATER_EQUAL"] = 305] = "TOKEN_GREATER_EQUAL";
    TokenOperatorType[TokenOperatorType["TOKEN_INCREMENT"] = 306] = "TOKEN_INCREMENT";
    TokenOperatorType[TokenOperatorType["TOKEN_DECREMENT"] = 307] = "TOKEN_DECREMENT";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_LEFT"] = 308] = "TOKEN_SHIFT_LEFT";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_RIGHT"] = 309] = "TOKEN_SHIFT_RIGHT";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_RIGHT_ASSIGN"] = 310] = "TOKEN_SHIFT_RIGHT_ASSIGN";
    TokenOperatorType[TokenOperatorType["TOKEN_SHIFT_LEFT_ASSIGN"] = 311] = "TOKEN_SHIFT_LEFT_ASSIGN";
    TokenOperatorType[TokenOperatorType["TOKEN_LEFT_BRACE"] = 312] = "TOKEN_LEFT_BRACE";
    TokenOperatorType[TokenOperatorType["TOKEN_RIGHT_BRACE"] = 313] = "TOKEN_RIGHT_BRACE";
    TokenOperatorType[TokenOperatorType["TOKEN_LEFT_BRACKET"] = 314] = "TOKEN_LEFT_BRACKET";
    TokenOperatorType[TokenOperatorType["TOKEN_RIGHT_BRACKET"] = 315] = "TOKEN_RIGHT_BRACKET";
    TokenOperatorType[TokenOperatorType["TOKEN_COLON"] = 316] = "TOKEN_COLON";
    TokenOperatorType[TokenOperatorType["TOKEN_COLON_COLON"] = 317] = "TOKEN_COLON_COLON";
    TokenOperatorType[TokenOperatorType["TOKEN_COMMA"] = 318] = "TOKEN_COMMA";
    TokenOperatorType[TokenOperatorType["TOKEN_LEFT_PAREN"] = 319] = "TOKEN_LEFT_PAREN";
    TokenOperatorType[TokenOperatorType["TOKEN_RIGHT_PAREN"] = 320] = "TOKEN_RIGHT_PAREN";
    TokenOperatorType[TokenOperatorType["TOKEN_PLUS"] = 321] = "TOKEN_PLUS";
    TokenOperatorType[TokenOperatorType["TOKEN_MINUS"] = 322] = "TOKEN_MINUS";
    TokenOperatorType[TokenOperatorType["TOKEN_MULTIPLY"] = 323] = "TOKEN_MULTIPLY";
    TokenOperatorType[TokenOperatorType["TOKEN_DIVIDE"] = 324] = "TOKEN_DIVIDE";
    TokenOperatorType[TokenOperatorType["TOKEN_DIVIDE_INT"] = 325] = "TOKEN_DIVIDE_INT";
    TokenOperatorType[TokenOperatorType["TOKEN_MOD"] = 326] = "TOKEN_MOD";
    TokenOperatorType[TokenOperatorType["TOKEN_POWER"] = 327] = "TOKEN_POWER";
    TokenOperatorType[TokenOperatorType["TOKEN_ASSIGN"] = 328] = "TOKEN_ASSIGN";
    TokenOperatorType[TokenOperatorType["TOKEN_LESS"] = 329] = "TOKEN_LESS";
    TokenOperatorType[TokenOperatorType["TOKEN_GREATER"] = 330] = "TOKEN_GREATER";
    TokenOperatorType[TokenOperatorType["TOKEN_NOT"] = 331] = "TOKEN_NOT";
    TokenOperatorType[TokenOperatorType["TOKEN_AND_BIT"] = 332] = "TOKEN_AND_BIT";
    TokenOperatorType[TokenOperatorType["TOKEN_OR_BIT"] = 333] = "TOKEN_OR_BIT";
    TokenOperatorType[TokenOperatorType["TOKEN_XOR_BIT"] = 334] = "TOKEN_XOR_BIT";
    TokenOperatorType[TokenOperatorType["TOKEN_MEMBER"] = 335] = "TOKEN_MEMBER";
    TokenOperatorType[TokenOperatorType["TOKEN_RANGE"] = 336] = "TOKEN_RANGE";
})(TokenOperatorType || (exports.TokenOperatorType = TokenOperatorType = {}));
;
var TokenOtherType;
(function (TokenOtherType) {
    TokenOtherType[TokenOtherType["TOKEN_ERROR"] = 0] = "TOKEN_ERROR";
    TokenOtherType[TokenOtherType["TOKEN_EOF"] = 1] = "TOKEN_EOF";
})(TokenOtherType || (exports.TokenOtherType = TokenOtherType = {}));
;
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
