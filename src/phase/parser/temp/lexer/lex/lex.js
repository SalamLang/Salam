"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.lex = lex;
const token_1 = require("./../tokenizer/token");
const number_1 = require("./number");
const identifier_1 = require("./identifier");
const utf8_1 = require("./utf8");
const type_1 = require("./../tokenizer/type");
const single_1 = require("./comment/single");
const string_1 = require("./string");
const multi_1 = require("./comment/multi");
const type_2 = require("./../tokenizer/type");
function lex(lexer) {
    while (lexer.index < lexer.source.length) {
        const char = lexer.currentChar;
        switch (char) {
            // End of file
            case '\0':
                lexer.advance();
                break;
            // New line
            case '\n':
                lexer.line++;
                lexer.column = 0;
                lexer.advance();
                break;
            // Whitespace
            case '\a': // Alert
            case '\b': // Backspace
            case '\f': // Form feed
            case '\r': // Carriage return
            case '\t': // Horizontal tab
            case '\v': // Vertical tab
            case ' ': // Space
                lexer.advance();
                break;
            case '+':
            case '﹢':
            case '＋':
                lexer.readDoubleToken(char);
                break;
            case '-':
            case '−':
                lexer.readDoubleToken(char);
                break;
            case '*':
            case '×':
                lexer.readDoubleToken(char);
                break;
            case '/':
            case '÷':
                if (lexer.nextChar === '/') {
                    (0, single_1.lexerLexReadComment)(lexer);
                }
                else if (lexer.nextChar === '*') {
                    (0, multi_1.lexerLexReadBlockComment)(lexer);
                }
                else {
                    lexer.advance();
                    lexer.pushToken(new token_1.Token(type_1.operatorTypeMaps[char], lexer.getLocation()));
                }
                break;
            case '&':
                lexer.readDoubleToken(char);
                break;
            case '|':
                lexer.readDoubleToken(char);
                break;
            case ':':
                lexer.readDoubleToken(char);
                break;
            case '|':
                lexer.readDoubleToken(char);
                break;
            case '.':
                lexer.readDoubleToken(char);
                break;
            case '=':
                lexer.readThreeToken(char, char);
                break;
            case '-':
                if (lexer.nextChar === '>') {
                    const nextChar = lexer.nextChar;
                    lexer.advance();
                    lexer.advance();
                    lexer.pushToken(new token_1.Token(type_1.operatorTypeMaps[char + nextChar], lexer.getLocation()));
                }
                else {
                    lexer.advance();
                    lexer.pushToken(new token_1.Token(type_1.operatorTypeMaps[char], lexer.getLocation()));
                }
                break;
            case '<':
                // <: TokenType.TOKEN_LESS
                // <<: TokenType.TOKEN_SHIFT_LEFT
                // <=: TokenType.TOKEN_LESS_EQUAL
                lexer.readThreeOrToken(char, '=');
                break;
            case '>':
                // >: TokenType.TOKEN_GREATER
                // >>: TokenType.TOKEN_SHIFT_RIGHT
                // >=: TokenType.TOKEN_GREATER_EQUAL
                lexer.readThreeOrToken(char, '=');
                break;
            case '~':
            case '{':
            case '}':
            case '[':
            case ']':
            case ',':
            case '(':
            case ')':
            case '%':
            case '^':
            case '?':
            case '!':
            case '⩵':
                lexer.pushToken(new token_1.Token(type_1.operatorTypeMaps[char], lexer.getLocation()));
                lexer.advance();
                break;
            default:
                if (string_1.stringOpenings.includes(char)) {
                    (0, string_1.lexerLexString)(lexer, char);
                }
                else if ((0, utf8_1.isUtf8Alpha)(char)) {
                    (0, identifier_1.lexerLexIdentifier)(lexer);
                }
                else if ((0, utf8_1.isUtf8Number)(char)) {
                    (0, number_1.lexerLexNumber)(lexer);
                }
                else {
                    lexer.pushError(`Unexpected character '${char}'`);
                    lexer.advance();
                }
        }
        lexer.pushToken(new token_1.Token(type_2.TokenOtherType.TOKEN_EOF, lexer.getLocation()));
    }
}
;
