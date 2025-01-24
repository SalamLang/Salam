import { Lexer } from "./lexer";
import { isUtf8Alpha } from './utf8';
import { Token } from "./../tokenizer/token";
import { TokenType } from "./../tokenizer/type";
import { TokenData, TokenDataType } from "./../tokenizer/data";
import { keywordMapsValues } from './../tokenizer/keyword';

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

    for (const keywordMap of keywordMapsValues) {
        if (keywordMap.data && keywordMap.data[lexer.language.id].includes(value)) {
            console.log(keywordMap.data, value, lexer.language.id, keywordMap.data[lexer.language.id] || "?", keywordMap.id);

            const token: Token = new Token(TokenType.TOKEN_KEYWORD, lexer.getLocation(), data);
            token.setKeywordType(keywordMap.id);
            lexer.pushToken(token);
            return;
        }
    }

    const token: Token = new Token(TokenType.TOKEN_KEYWORD, lexer.getLocation(), data);
    lexer.pushToken(token);
};
