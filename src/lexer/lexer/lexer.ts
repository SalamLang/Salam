import { Token } from "./../tokenizer/token";
import { stringify } from './../../serializer';
import { TokenData } from "./../tokenizer/data";
import { TokenType } from './../tokenizer/type';
import { TokenDataType } from './../tokenizer/data';
import { operatorTypeMaps } from './../tokenizer/type';
import { TokenLocation } from './../tokenizer/location';
import { LanguageMap } from './../cli/language/language';

export class Lexer {
    source: string;
    index: number;
    line: number;
    column: number;
    tokens: Token[];
    selectedLanguage: LanguageMap;

    constructor(source: string, selectedLanguage: LanguageMap) {
        this.source = source;
        this.index = 0;
        this.line = 1;
        this.column = 0;
        this.tokens = [];
        this.selectedLanguage = selectedLanguage;
    }

    get currentChar(): string {
        return this.source[this.index] || '\0';
    }

    get previousChar(): string {
        return this.source[this.index - 1] || '\0';
    }

    get nextChar(): string {
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

    pushError(message: string) {
        console.error(`Error: ${message} at line ${this.line}, column ${this.column}.`);
        const tokenData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_STRING, message);
        const token = new Token(TokenType.TOKEN_ERROR, this.getLocation(), tokenData);
        this.pushToken(token);
    }

    pushToken(token: Token) {
        this.tokens.push(token);
    }

    getLocation(): TokenLocation {
        return new TokenLocation(
            this.index,
            1,
            this.line,
            this.column,
            this.line,
            this.column + 1
        );
    }

    readDoubleToken(char: string): void {
        const nextChar = this.nextChar;
        if (nextChar === char) {
            this.advance();
            this.advance();

            const token = new Token(operatorTypeMaps[char + char], this.getLocation());
            this.pushToken(token);
        } else {
            this.advance();

            const token = new Token(operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }

    readThreeOrToken(char: string, char2: string): void {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();

            const token = new Token(operatorTypeMaps[char + char2], this.getLocation());
            this.pushToken(token);
        } else if (nextChar === char) {
            this.advance();
            this.advance();

            if (this.nextChar === char2) {
                this.advance();
                const token = new Token(operatorTypeMaps[char + char + char2], this.getLocation());
                this.pushToken(token);
            } else {
                const token = new Token(operatorTypeMaps[char + char], this.getLocation());
                this.pushToken(token);
            }
        } else {
            this.advance();

            const token = new Token(operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }

    readThreeToken(char: string, char2: string): void {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();

            const token = new Token(operatorTypeMaps[char + char2], this.getLocation());
            this.pushToken(token);
        } else if (nextChar === char) {
            this.advance();
            this.advance();

            const token = new Token(operatorTypeMaps[char + char], this.getLocation());
            this.pushToken(token);
        } else {
            this.advance();

            const token = new Token(operatorTypeMaps[char], this.getLocation());
            this.pushToken(token);
        }
    }

    lex() {

    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(): string | object {
        const jsonArray = this.tokens.map(token => token.stringify(false));
        return stringify(jsonArray, true);
    }
}
