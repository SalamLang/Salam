import { Lexer } from "./lexer";
import { Token } from "./../tokenizer/token";
import { utf8Decode, isUtf8Alpha} from './utf8';

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

    const token: Token = {
        type: 'IDENTIFIER',
        dataType: 'IDENTIFIER',
        data: value,
        location: {
            index: lexer.index,
            line: lexer.line,
            columnStart: lexer.column,
            columnEnd: lexer.column + value.length
        }
    };
    lexer.pushToken(token);
};
