// import { isUtf8Alpha } from './utf8';
import { Token } from "./../tokenizer/token";
// import { lexerLexNumber } from './number';
// import { lexerLexIdentifier } from './identifier';
import { operatorTypeMaps } from './../tokenizer/type';
import { LanguageMap } from './../cli/language/language';
import { TokenType } from '../tokenizer/type';
import { TokenLocation } from '../tokenizer/location';
    
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

    readDoubleToken(char: string, tokenType: TokenType, doubleTokenType: TokenType): void {
        const nextChar = this.nextChar;
        if (nextChar === char) {
            this.advance();
            this.advance();

            const token = new Token(doubleTokenType, this.getLocation());
            this.pushToken(token);
        } else {
            this.advance();

            const token = new Token(tokenType, this.getLocation());
            this.pushToken(token);
        }
    }

    readThreeToken(char: string, char2: string, tokenType: TokenType, doubleTokenType: TokenType, thirdTokenType: TokenType): void {
        const nextChar = this.nextChar;
        if (nextChar === char2) {
            this.advance();
            this.advance();

            const token = new Token(thirdTokenType, this.getLocation());
            this.pushToken(token);
        } else if (nextChar === char) {
            this.advance();
            this.advance();

            const token = new Token(doubleTokenType, this.getLocation());
            this.pushToken(token);
        } else {
            this.advance();

            const token = new Token(tokenType, this.getLocation());
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
                    this.readDoubleToken(char, TokenType.TOKEN_PLUS, TokenType.TOKEN_INCREMENT);
                    break;

                case '-':
                case '−':
                        this.readDoubleToken(char, TokenType.TOKEN_MINUS, TokenType.TOKEN_DECREMENT);
                    break;

                case '*':
                case '×':
                    this.readDoubleToken(char, TokenType.TOKEN_MULTIPLY, TokenType.TOKEN_POWER);
                break;

                case '/':
                case '÷':
                    this.readDoubleToken(char, TokenType.TOKEN_DIVIDE, TokenType.TOKEN_DIVIDE_INT);
                break;

                case '&':
                    this.readDoubleToken(char, TokenType.TOKEN_AND_BIT, TokenType.TOKEN_AND_AND);
                    break;

                case '|':
                    this.readDoubleToken(char, TokenType.TOKEN_OR_BIT, TokenType.TOKEN_OR_OR);
                    break;

                case '=':
                    this.readDoubleToken(char, TokenType.TOKEN_EQUAL, TokenType.TOKEN_ASSIGN);
                    break;

                case '<':
                    this.readThreeToken(char, '=', TokenType.TOKEN_LESS, TokenType.TOKEN_SHIFT_LEFT, TokenType.TOKEN_LESS_EQUAL);
                    break;

                case '>':
                    this.readThreeToken(char, '=', TokenType.TOKEN_GREATER, TokenType.TOKEN_SHIFT_RIGHT, TokenType.TOKEN_GREATER_EQUAL);
                    break;
        
                // case '~':
                case '{':
                case '}':
                case '[':
                case ']':
                case ':':
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
            }
        }
    }

    print(): void {
        console.log(this.stringify());
    }

    stringify(): string {
        return JSON.stringify(this.tokens.map(token => token.stringify()));
    }
}
