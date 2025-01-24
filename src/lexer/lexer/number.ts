import { Lexer } from "./lexer";
import { Token } from "./../tokenizer/token";
import { utf8Decode} from './utf8';

export function lexerLexNumber(lexer: Lexer) {
    let value = '';
    let isFloat = false;

    while (lexer.currentChar !== '\0') {
        const { char, newIndex } = utf8Decode(lexer.source, lexer.index);
        if (char === '.' && !isFloat) {
            isFloat = true;
        } else if (!/\d/.test(char)) {
            lexer.index = newIndex - 1; // Step back
            break;
        }
        value += char;
        lexer.index = newIndex;
    }

    const token: Token = {
        type: isFloat ? 'NUMBER_FLOAT' : 'NUMBER_INT',
        dataType: isFloat ? 'NUMBER_FLOAT' : 'NUMBER_INT',
        data: isFloat ? parseFloat(value) : parseInt(value, 10),
        location: {
            index: lexer.index,
            line: lexer.line,
            columnStart: lexer.column,
            columnEnd: lexer.column + value.length
        }
    };
    lexer.pushToken(token);
}
