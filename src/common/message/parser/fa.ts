import { ParserMessageKeys, ParserMessageStruct } from './../message';

export const faMessages: ParserMessageStruct = {
    [ParserMessageKeys.PARSER_PREFIX]: "خطای گرامری: ",
    [ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION_STATEMENT]: "ناتوان در تجزیه و تحلیل اظهارات تابع.",
    [ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION]: "ناتوان در تجزیه و تحلیل تابع.",
    [ParserMessageKeys.PARSER_FAILED_TO_PARSE_LAYOUT_ELEMENT]: "ناتوان در تجزیه و تحلیل عنصر طرح.",
    [ParserMessageKeys.PARSER_DUPLICATE_LAYOUT_DEFINITION]: "تعریف طرح تکراری است، نمی‌توان بیش از یک تعریف طرح در یک برنامه داشت.",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_PROGRAM]: "نشانه غیرمنتظره در برنامه، نشانه فعلی '{0}'",

    [ParserMessageKeys.PARSER_UNEXPECTED_END_OF_TOKENS_IN_LAYOUT_ATTRIBUTE]: "پایان غیرمنتظره نشانه‌ها در ویژگی طرح",

    [ParserMessageKeys.PARSER_DUPLICATE_ATTRIBUTE_IN_LAYOUT]: "ویژگی تکراری '{0}' در طرح",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_AS_ELEMENT_NAME]: "نشانه غیرمنتظره به عنوان نام عنصر، نشانه فعلی '{0}'",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT]: "نشانه غیرمنتظره در طرح، نشانه فعلی '{0}'",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT_AS_ATTRIBUTE]: "نشانه غیرمنتظره در طرح به عنوان ویژگی، نشانه فعلی '{0}'",

    [ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID_IDENTIFIER]: "نام تابع معتبر نیست، باید یک رشته شناسه باشد.",
    [ParserMessageKeys.PARSER_FUNCTION_NAME_IS_RESERVED_IN_SALAM]: "نام تابع معتبر نیست، این نام در Salam از قبل رزرو شده است، باید نام دیگری انتخاب کنید.",
    [ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID]: "نام تابع معتبر نیست.",
    [ParserMessageKeys.PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID]: "پارامترهای تابع معتبر نیستند.",
    [ParserMessageKeys.PARSER_FUNCTION_BODY_IS_NOT_VALID]: "بدنه تابع معتبر نیست.",

    [ParserMessageKeys.PARSER_EXPECTED_TOKEN_TYPE_BUT_GOT]: "نوع نشانه {0} انتظار می‌رفت، اما {1} دریافت شد.",
};
