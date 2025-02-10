import { Lexer } from "./lexer";
import { Token } from "./../tokenizer/token";
import { TokenValueType } from "./../tokenizer/type";
import { isUtf8Number, toEnglishDigit } from './utf8';
import { TokenData, TokenDataType } from "./../tokenizer/data";

export function lexerLexNumber(lexer: Lexer) {
    let value = '';
    let isFloat = false;

    while (lexer.currentChar !== '\0') {
        const char: string = lexer.currentChar;
        if (char === '.' && !isFloat) {
            isFloat = true;
        } else if (! isUtf8Number(char)) {
            break;
        }

        value += toEnglishDigit(char);
        lexer.advance();
    }

    const data: TokenData = new TokenData(
        isFloat ? TokenDataType.TOKEN_DATA_TYPE_FLOAT : TokenDataType.TOKEN_DATA_TYPE_INT,
        isFloat ? parseFloat(value) : parseInt(value, 10)
    );
    const token: Token = new Token(
        isFloat ? TokenValueType.TOKEN_NUMBER_FLOAT : TokenValueType.TOKEN_NUMBER_INT,
        lexer.getLocation(),
        value,
        data
    );
    lexer.pushToken(token);
};
