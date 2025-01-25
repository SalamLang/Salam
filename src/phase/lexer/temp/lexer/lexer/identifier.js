"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.lexerLexIdentifier = lexerLexIdentifier;
const utf8_1 = require("./utf8");
const token_1 = require("./../tokenizer/token");
const type_1 = require("./../tokenizer/type");
const data_1 = require("./../tokenizer/data");
const keyword_1 = require("./../tokenizer/keyword");
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
    for (const keywordMap of keyword_1.keywordMapsValues) {
        if (keywordMap.data && keywordMap.data[lexer.language.id].includes(value)) {
            const token = new token_1.Token(keywordMap.id, lexer.getLocation(), data);
            lexer.pushToken(token);
            return;
        }
    }
    const token = new token_1.Token(type_1.TokenKeywordType.TOKEN_IDENTIFIER, lexer.getLocation(), data);
    lexer.pushToken(token);
}
;
