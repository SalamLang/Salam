import { LexerMessageKeys, LexerMessageStruct } from './../message';

export const faMessages: LexerMessageStruct = {
    [LexerMessageKeys.LEXER_INVALID_UNEXPECTED_CHAR]: "کاراکتر غیرمنتظره '{0}'",
    [LexerMessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]: "بلوک کامنت چندخطی بسته نشده است",
};
