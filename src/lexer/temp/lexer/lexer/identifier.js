"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.lexerLexIdentifier = lexerLexIdentifier;
const token_1 = require("./../tokenizer/token");
const type_1 = require("./../tokenizer/type");
const utf8_1 = require("./utf8");
const data_1 = require("./../tokenizer/data");
function lexerLexIdentifier(lexer) {
    let value = '';
    while (lexer.currentChar !== '\0') {
        const char = lexer.currentChar;
        if (!(0, utf8_1.isUtf8Alpha)(char)) {
            break;
        }
        value += char;
        lexer.advance();
    }
    const data = new data_1.TokenData(data_1.TokenDataType.TOKEN_DATA_TYPE_STRING, value);
    const token = new token_1.Token(type_1.TokenType.TOKEN_IDENTIFIER, lexer.getLocation(), data);
    lexer.pushToken(token);
}
;
