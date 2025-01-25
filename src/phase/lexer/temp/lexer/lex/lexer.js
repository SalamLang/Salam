"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.Lexer = void 0;
const token_1 = require("../tokenizer/token");
const serializer_1 = require("../../serializer");
const data_1 = require("../tokenizer/data");
const type_1 = require("../tokenizer/type");
const data_2 = require("../tokenizer/data");
const type_2 = require("../tokenizer/type");
const location_1 = require("../tokenizer/location");
class Lexer {
    constructor(source, selectedLanguage, fileName, absoluteDirPath) {
        this.source = source;
        this.index = 0;
        this.line = 1;
        this.column = 0;
        this.tokens = [];
        this.language = selectedLanguage;
        this.fileName = fileName;
        this.absoluteDirPath = absoluteDirPath;
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
        const token = new token_1.Token(type_1.TokenOtherType.TOKEN_ERROR, this.getLocation(), tokenData);
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
            const token = new token_1.Token(type_2.operatorTypeMaps[char + char], this.getLocation());
            this.pushToken(token);
        }
        else {
            this.advance();
            const token = new token_1.Token(type_2.operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }
    readThreeOrToken(char, char2) {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_2.operatorTypeMaps[char + char2], this.getLocation());
            this.pushToken(token);
        }
        else if (nextChar === char) {
            this.advance();
            this.advance();
            if (this.nextChar === char2) {
                this.advance();
                const token = new token_1.Token(type_2.operatorTypeMaps[char + char + char2], this.getLocation());
                this.pushToken(token);
            }
            else {
                const token = new token_1.Token(type_2.operatorTypeMaps[char + char], this.getLocation());
                this.pushToken(token);
            }
        }
        else {
            this.advance();
            const token = new token_1.Token(type_2.operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }
    readThreeToken(char, char2) {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_2.operatorTypeMaps[char + char2], this.getLocation());
            this.pushToken(token);
        }
        else if (nextChar === char) {
            this.advance();
            this.advance();
            const token = new token_1.Token(type_2.operatorTypeMaps[char + char], this.getLocation());
            this.pushToken(token);
        }
        else {
            this.advance();
            const token = new token_1.Token(type_2.operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
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
