import { LexerMessageKeys, LexerMessageStruct } from './../message';

export const enMessages: LexerMessageStruct = {
    [LexerMessageKeys.LEXER_INVALID_UNEXPECTED_CHAR]: "Unexpected character '{0}'",
    [LexerMessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]: "Unterminated multi-line comment block",
};
