import { Lexer } from "./lexer";
import { Token } from "./../tokenizer/token";
import { TokenType } from "./../tokenizer/type";
import { isUtf8Number } from './utf8';
import { TokenData, TokenDataType } from "./../tokenizer/data";

export function lexerLexNumber(lexer: Lexer) {
    let value = '';
    let isFloat = false;

    while (lexer.currentChar !== '\0') {
        const char = lexer.currentChar;
        if (char === '.' && !isFloat) {
            isFloat = true;
        } else if (!isUtf8Number(char)) {
            lexer.retreat();
            break;
        }

        value += char;
        lexer.advance();
    }

    const data = new TokenData(
        isFloat ? TokenDataType.TOKEN_DATA_TYPE_FLOAT : TokenDataType.TOKEN_DATA_TYPE_INT, 
        isFloat ? parseFloat(value) : parseInt(value, 10)
    );
    const token: Token = new Token(TokenType.TOKEN_IDENTIFIER, lexer.getLocation(), data);
    lexer.pushToken(token);
};
