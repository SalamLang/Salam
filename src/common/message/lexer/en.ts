import { LexerMessageKeys } from './../message';

export const enMessages: Record<LexerMessageKeys, string> = {
    [LexerMessageKeys.LEXER_PREFIX]: "Syntax error: ",
    [LexerMessageKeys.LEXER_INVALID_UNEXPECTED_CHAR]: "Unexpected character '{0}'",
    [LexerMessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]: "Unterminated multi-line comment block",
    [LexerMessageKeys.LEXER_UNCLOSED_STRING_LITERAL]: "String literal starting with '{0}' is unclosed",
};
