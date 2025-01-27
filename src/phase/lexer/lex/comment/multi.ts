import { Lexer } from './../lexer';
import { lexerMessages } from './../../../../common/message/lexer/lexer';
import { messageRenderer, LexerMessageKeys } from './../../../../common/message/message';

export function lexerLexReadBlockComment(lexer: Lexer) {
    lexer.advance(); // Skip the first '/'
    lexer.advance(); // Skip the first '*'

    let blockCommentDepth = 1;

    while (lexer.currentChar !== '\0' && blockCommentDepth > 0) {
        if (lexer.currentChar === '/' && lexer.nextChar === '*') {
            blockCommentDepth++;
            lexer.advance();
        } else if (lexer.currentChar === '*' && lexer.nextChar === '/') {
            blockCommentDepth--;
            lexer.advance();
        }
        lexer.advance();
    }

    if (blockCommentDepth > 0) {
        lexer.pushError(messageRenderer(lexerMessages[lexer.language.id][LexerMessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]));
    }
}
