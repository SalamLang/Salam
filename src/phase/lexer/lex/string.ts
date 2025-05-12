import { Lexer } from "./lexer"; 
import { Token } from "./../tokenizer/token";
import { TokenValueType } from "./../tokenizer/type";
import { TokenData, TokenDataType } from "./../tokenizer/data";
// import { lexerMessages } from './../../../common/message/lexer/lexer';
// import { messageRenderer } from './../../../common/message/message';

export const stringOpenings: string[] = [
    '"',
    "'",
    '`',
    '“',
    '«',
    '‘',
];

export const stringClosings: string[] = [
    '"',
    "'",
    '`',
    '”',
    '»',
    '’',
];

export function lexerLexString(lexer: Lexer, opening: string): void {
    let value = '';

    lexer.advance();

    const closing: string | undefined = stringClosings[stringOpenings.indexOf(opening)];

    while (lexer.currentChar !== '\0') {
        if (lexer.currentChar === closing) {
            lexer.advance();
            break;
        }

        if (lexer.currentChar === '\\') {
            lexer.advance();
            value += '\\';
            switch (lexer.currentChar) {
                case 'n': value += 'n'; break;
                case 't': value += 't'; break;
                case 'r': value += 'r'; break;
                case '\\': value += '\\'; break;
                case '"': value += '"'; break;
                case "'": value += "'"; break;
                case '`': value += '`'; break;
                default:
                    value += lexer.currentChar;
            }
        } else {
            value += lexer.currentChar;
        }

        lexer.advance();
    }

    // TODO: If we reach here and the string isn't properly closed, you can handle the error
    // Uncomment if you want error handling for unclosed strings
    // if (lexer.currentChar !== closing) {
    //     lexer.pushError(lexerMessageRenderer(lexer.language.id, LexerMessageKeys.LEXER_UNCLOSED_STRING_LITERAL, opening));
    //     return;
    // }

    const tokenData: TokenData = new TokenData(TokenDataType.TOKEN_DATA_TYPE_STRING, value);
    const token: Token = new Token(TokenValueType.TOKEN_STRING, lexer.getLocation(), value, tokenData);
    lexer.pushToken(token);
};
