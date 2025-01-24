"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Lexer = void 0;
const token_1 = require("./../tokenizer/token");
const data_1 = require("./../tokenizer/data");
const data_2 = require("./../tokenizer/data");
const number_1 = require("./number");
const identifier_1 = require("./identifier");
const string_1 = require("./string");
const utf8_1 = require("./utf8");
const type_1 = require("./../tokenizer/type");
const type_2 = require("./../tokenizer/type");
const location_1 = require("./../tokenizer/location");
const serializer_1 = require("./../../serializer");
class Lexer {
    constructor(source, selectedLanguage) {
        this.source = source;
        this.index = 0;
        this.line = 1;
        this.column = 0;
        this.tokens = [];
        this.selectedLanguage = selectedLanguage;
    }
    get currentChar() {
        return this.source[this.index] || '\0';
    }
    get previousChar() {
        return this.source[this.index - 1] || '\0';
    }
    get nextChar() {
        return this.source[this.index + 1] || '\0';
    }
    advance() {
        this.index++;
        this.column++;
    }
    retreat() {
        this.index--;
        this.column--;
    }
    pushError(message) {
        console.error(`Error: ${message} at line ${this.line}, column ${this.column}.`);
        const tokenData = new data_1.TokenData(data_2.TokenDataType.TOKEN_DATA_TYPE_STRING, message);
        const token = new token_1.Token(type_2.TokenType.TOKEN_ERROR, this.getLocation(), tokenData);
        this.pushToken(token);
    }
    pushToken(token) {
        this.tokens.push(token);
    }
    getLocation() {
        return new location_1.TokenLocation(this.index, 1, this.line, this.column, this.line, this.column + 1);
    }
    readDoubleToken(char) {
        const nextChar = this.nextChar;
        if (nextChar === char) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char + char], this.getLocation());
            this.pushToken(token);
        }
        else {
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }
    readThreeOrToken(char, char2) {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char + char2], this.getLocation());
            this.pushToken(token);
        }
        else if (nextChar === char) {
            this.advance();
            this.advance();
            if (this.nextChar === char2) {
                this.advance();
                const token = new token_1.Token(type_1.operatorTypeMaps[char + char + char2], this.getLocation());
                this.pushToken(token);
            }
            else {
                const token = new token_1.Token(type_1.operatorTypeMaps[char + char], this.getLocation());
                this.pushToken(token);
            }
        }
        else {
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }
    readThreeToken(char, char2) {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char + char2], this.getLocation());
            this.pushToken(token);
        }
        else if (nextChar === char) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char + char], this.getLocation());
            this.pushToken(token);
        }
        else {
            this.advance();
            const token = new token_1.Token(type_1.operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }
    lex() {
        while (this.index < this.source.length) {
            const char = this.currentChar;
            switch (char) {
                // End of file
                case '\0':
                    this.advance();
                    break;
                // New line
                case '\n':
                    this.line++;
                    this.column = 0;
                    this.advance();
                    break;
                // Whitespace
                case '\a': // Alert
                case '\b': // Backspace
                case '\f': // Form feed
                case '\r': // Carriage return
                case '\t': // Horizontal tab
                case '\v': // Vertical tab
                case ' ': // Space
                    this.advance();
                    break;
                case '+':
                case '﹢':
                case '＋':
                    this.readDoubleToken(char);
                    break;
                case '-':
                case '−':
                    this.readDoubleToken(char);
                    break;
                case '*':
                case '×':
                    this.readDoubleToken(char);
                    break;
                case '/':
                case '÷':
                    this.readDoubleToken(char);
                    break;
                case '&':
                    this.readDoubleToken(char);
                    break;
                case '|':
                    this.readDoubleToken(char);
                    break;
                case ':':
                    this.readDoubleToken(char);
                    break;
                case '|':
                    this.readDoubleToken(char);
                    break;
                case '.':
                    this.readDoubleToken(char);
                    break;
                case '=':
                    this.readThreeToken(char, char);
                    break;
                case '-':
                    if (this.nextChar === '>') {
                        const nextChar = this.nextChar;
                        this.advance();
                        this.advance();
                        const token = new token_1.Token(type_1.operatorTypeMaps[char + nextChar], this.getLocation());
                        this.pushToken(token);
                    }
                    else {
                        this.advance();
                        const token = new token_1.Token(type_1.operatorTypeMaps[char], this.getLocation());
                        this.pushToken(token);
                    }
                    break;
                case '<':
                    // <: TokenType.TOKEN_LESS
                    // <<: TokenType.TOKEN_SHIFT_LEFT
                    // <=: TokenType.TOKEN_LESS_EQUAL
                    this.readThreeOrToken(char, '=');
                    break;
                case '>':
                    // >: TokenType.TOKEN_GREATER
                    // >>: TokenType.TOKEN_SHIFT_RIGHT
                    // >=: TokenType.TOKEN_GREATER_EQUAL
                    this.readThreeOrToken(char, '=');
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
                    const token = new token_1.Token(type_1.operatorTypeMaps[char], this.getLocation());
                    this.pushToken(token);
                    this.advance();
                    break;
                default:
                    if (string_1.stringOpenings.includes(char)) {
                        (0, string_1.lexerLexString)(this, char);
                    }
                    else if ((0, utf8_1.isUtf8Alpha)(char)) {
                        (0, identifier_1.lexerLexIdentifier)(this);
                    }
                    else if ((0, utf8_1.isUtf8Number)(char)) {
                        (0, number_1.lexerLexNumber)(this);
                    }
                    else {
                        console.log(`Error: Unexpected character '${char}' at line ${this.line}, column ${this.column}.`);
                        this.advance();
                    }
            }
        }
    }
    print() {
        console.log(this.stringify());
    }
    stringify() {
        const jsonArray = this.tokens.map(token => token.stringify(false));
        return (0, serializer_1.stringify)(jsonArray, true);
    }
}
exports.Lexer = Lexer;
