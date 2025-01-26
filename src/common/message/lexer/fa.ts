import { MessageKeys, MessageStruct } from './../message';

export const faMessages: MessageStruct = {
    [MessageKeys.LEXER_INVALID_UNEXPECTED_CHAR]: "کاراکتر غیرمنتظره '{0}'",
    [MessageKeys.LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK]: "بلوک کامنت چندخطی بسته نشده است",
};
