"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.lexerLexNumber = lexerLexNumber;
const token_1 = require("./../tokenizer/token");
const type_1 = require("./../tokenizer/type");
const utf8_1 = require("./utf8");
const data_1 = require("./../tokenizer/data");
function lexerLexNumber(lexer) {
    let value = '';
    let isFloat = false;
    while (lexer.currentChar !== '\0') {
        const char = lexer.currentChar;
        if (char === '.' && !isFloat) {
            isFloat = true;
        }
        else if (!(0, utf8_1.isUtf8Number)(char)) {
            break;
        }
        value += (0, utf8_1.toEnglishDigit)(char);
        lexer.advance();
    }
    const data = new data_1.TokenData(isFloat ? data_1.TokenDataType.TOKEN_DATA_TYPE_FLOAT : data_1.TokenDataType.TOKEN_DATA_TYPE_INT, isFloat ? parseFloat(value) : parseInt(value, 10));
    const token = new token_1.Token(isFloat ? type_1.TokenTypeCombined.TOKEN_NUMBER_FLOAT : type_1.TokenTypeCombined.TOKEN_NUMBER_INT, lexer.getLocation(), data);
    lexer.pushToken(token);
}
;
