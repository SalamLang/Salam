import { MessageKeys, MessageStruct, LexerMessageKeys } from './../message';

export const enMessages: MessageStruct<LexerMessageKeys> = {
    [MessageKeys.LEXER_PREFIX]: "Syntax error: ",
    [MessageKeys.LEXER_INVALID_UNEXPECTED_CHAR]: "Unexpected character '{0}'",
    [MessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]: "Unterminated multi-line comment block",
    [MessageKeys.LEXER_UNCLOSED_STRING_LITERAL]: "String literal starting with '{0}' is unclosed",
};
