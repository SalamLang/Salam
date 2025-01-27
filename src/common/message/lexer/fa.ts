import { MessageKeys, MessageStruct, LexerMessageKeys } from './../message';

export const faMessages: MessageStruct<LexerMessageKeys> = {
    [MessageKeys.LEXER_PREFIX]: 'خطای نحوی: ',
    [MessageKeys.LEXER_INVALID_UNEXPECTED_CHAR]: "کاراکتر غیرمنتظره '{0}'",
    [MessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]: "بلوک کامنت چندخطی بسته نشده است",
    [MessageKeys.LEXER_UNCLOSED_STRING_LITERAL]: "رشته شروع شده با '{0}' بسته نشده است",
};
