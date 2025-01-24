import { Token } from "./../tokenizer/token";
import { TokenData } from "./../tokenizer/data";
import { lexerLexNumber } from './number';
import { lexerLexIdentifier } from './identifier';
import { lexerLexString, stringOpenings } from './string';
import { isUtf8Alpha, isUtf8Number } from './utf8';
import { operatorTypeMaps } from './../tokenizer/type';
import { LanguageMap } from './../cli/language/language';
import { TokenType } from './../tokenizer/type';
import { TokenLocation } from './../tokenizer/location';
import { stringify } from './../../serializer';

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
                case '\a':  // Alert
                case '\b':  // Backspace
                case '\f':  // Form feed
                case '\r':  // Carriage return
                case '\t':  // Horizontal tab
                case '\v':  // Vertical tab
                case ' ':   // Space
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

                        const token = new Token(operatorTypeMaps[char + nextChar], this.getLocation());
                        this.pushToken(token);
                    } else {
                        this.advance();

                        const token = new Token(operatorTypeMaps[char], this.getLocation());
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
                    const token = new Token(operatorTypeMaps[char], this.getLocation());
                    this.pushToken(token);
                    this.advance();
                    break;
                
                default:
                    if (stringOpenings.includes(char)) {
                        lexerLexString(this, char);
                    } else if (isUtf8Alpha(char)) {
                        lexerLexIdentifier(this);
                    } else if (isUtf8Number(char)) {
                        lexerLexNumber(this);
                    } else {
                        console.log(`Error: Unexpected character '${char}' at line ${this.line}, column ${this.column}.`);
                        this.advance();
                    }
            }
        }
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(): string | object {
        const jsonArray = this.tokens.map(token => token.stringify(false));
        return stringify(jsonArray, true);
    }
}
