import { Lexer } from "./lexer";
import { isUtf8Alpha } from './utf8';
import { Token } from "./../tokenizer/token";
import { TokenType } from "./../tokenizer/type";
import { TokenData, TokenDataType } from "./../tokenizer/data";

export function lexerLexIdentifier(lexer: Lexer) {
    let value = '';

    while (lexer.currentChar !== '\0') {
        const char = lexer.currentChar;

        if (!isUtf8Alpha(char)) {
            break;
        }
        value += char;
        lexer.advance();
    }

    const data = new TokenData(TokenDataType.TOKEN_DATA_TYPE_STRING, value);
    const token: Token = new Token(TokenType.TOKEN_IDENTIFIER, lexer.getLocation(), data);
    lexer.pushToken(token);
};
