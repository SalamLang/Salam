import { Token } from "./../tokenizer/token";
import { Lexer } from "./lexer";
import { lexerLexNumber } from './number';
import { lexerLexIdentifier } from './identifier';
import { isUtf8Alpha, isUtf8Number } from './utf8';
import { operatorTypeMaps } from './../tokenizer/type';
import { lexerLexReadComment } from './comment/single';
import { lexerLexString, stringOpenings } from './string';
import { lexerLexReadBlockComment } from './comment/multi';

export function lex(lexer: Lexer): void {
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
            case '\a':  // Alert
            case '\b':  // Backspace
            case '\f':  // Form feed
            case '\r':  // Carriage return
            case '\t':  // Horizontal tab
            case '\v':  // Vertical tab
            case ' ':   // Space
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
                    lexerLexReadComment(lexer);
                } else if (lexer.nextChar === '*') {
                    lexerLexReadBlockComment(lexer);
                } else {
                    lexer.advance();
                    lexer.pushToken(new Token(operatorTypeMaps[char], lexer.getLocation()));
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

                    lexer.pushToken(new Token(operatorTypeMaps[char + nextChar], lexer.getLocation()));
                } else {
                    lexer.advance();

                    lexer.pushToken(new Token(operatorTypeMaps[char], lexer.getLocation()));
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
                lexer.pushToken(new Token(operatorTypeMaps[char], lexer.getLocation()));
                lexer.advance();
                break;
            
            default:
                if (stringOpenings.includes(char)) {
                    lexerLexString(lexer, char);
                } else if (isUtf8Alpha(char)) {
                    lexerLexIdentifier(lexer);
                } else if (isUtf8Number(char)) {
                    lexerLexNumber(lexer);
                } else {
                    console.log(`Error: Unexpected character '${char}' at line ${lexer.line}, column ${lexer.column}.`);
                    lexer.advance();
                }
        }
    }
};
