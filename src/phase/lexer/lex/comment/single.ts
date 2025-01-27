import { Lexer } from "./../lexer";

export function lexerLexReadComment(lexer: Lexer) {
    lexer.advance(); // Skip the first '/'
    lexer.advance(); // Skip the first '/'

    while (lexer.currentChar !== '\0' && lexer.currentChar !== '\n') {
        lexer.advance();
    }
}
