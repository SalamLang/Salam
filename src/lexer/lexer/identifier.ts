import { Lexer } from "./lexer";
import { Token } from "./../tokenizer/token";
import { TokenType } from "./../tokenizer/type";
import { utf8Decode, isUtf8Alpha} from './utf8';
import { TokenData, TokenDataType } from "./../tokenizer/data";

export function lexerLexIdentifier(lexer: Lexer) {
    let value = '';

    while (lexer.currentChar !== '\0') {
        const { char, newIndex } = utf8Decode(lexer.source, lexer.index);
        if (!isUtf8Alpha(char)) {
            lexer.index = newIndex - 1;
            break;
        }
        value += char;
        lexer.index = newIndex;
    }

    const data = new TokenData(TokenDataType.TOKEN_DATA_TYPE_STRING, value);
    const token: Token = new Token(TokenType.TOKEN_IDENTIFIER, lexer.getLocation(), data);
    lexer.pushToken(token);
};
