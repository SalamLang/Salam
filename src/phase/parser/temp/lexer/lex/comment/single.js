"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.lexerLexReadComment = lexerLexReadComment;
function lexerLexReadComment(lexer) {
    lexer.advance(); // Skip the first '/'
    lexer.advance(); // Skip the first '/'
    while (lexer.currentChar !== '\0' && lexer.currentChar !== '\n') {
        lexer.advance();
    }
}
