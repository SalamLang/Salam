"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.stringClosings = exports.stringOpenings = void 0;
exports.lexerLexString = lexerLexString;
const token_1 = require("./../tokenizer/token");
const type_1 = require("./../tokenizer/type");
const data_1 = require("./../tokenizer/data");
exports.stringOpenings = [
    '"',
    "'",
    '`',
    '“',
    '«',
    '‘',
];
exports.stringClosings = [
    '"',
    "'",
    '`',
    '”',
    '»',
    '’',
];
function lexerLexString(lexer, opening) {
    let value = '';
    lexer.advance();
    const closing = exports.stringClosings[exports.stringOpenings.indexOf(opening)];
    while (lexer.currentChar !== '\0') {
        if (lexer.currentChar === closing) {
            lexer.advance();
            break;
        }
        if (lexer.currentChar === '\\') {
            lexer.advance();
            // if (lexer.currentChar === '"' || lexer.currentChar === "'" || lexer.currentChar === '\\') {
            //     value += lexer.currentChar;
            // } else {
            //     value += '\\';
            // }
        }
        else {
            value += lexer.currentChar;
        }
        lexer.advance();
    }
    // If we reach here and the string isn't properly closed, you can handle the error
    // Uncomment if you want error handling for unclosed strings
    // if (lexer.currentChar !== closing) {
    //     lexer.pushError(`Unclosed string literal starting with '${opening}'`);
    //     return;
    // }
    const tokenData = new data_1.TokenData(data_1.TokenDataType.TOKEN_DATA_TYPE_STRING, value);
    const token = new token_1.Token(type_1.TokenTypeCombined.TOKEN_STRING, lexer.getLocation(), tokenData);
    lexer.pushToken(token);
}
;
