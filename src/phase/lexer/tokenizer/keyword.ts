import { LanguageID } from './../../../common/language/language';
import { TokenKeywordDataType, TokenKeywordType, TokenType, TokenValueType } from "./type";

export interface KeywordMap {
    id: TokenType;
    data?: Record<LanguageID, string[]>;
};

export const keywordMaps: KeywordMap[] = [
    {
        id: TokenKeywordType.TOKEN_IDENTIFIER,
    },
    {
        id: TokenKeywordType.TOKEN_DEFER,
        data: {
            [LanguageID.LanguageEnglish]: ["defer"],
            [LanguageID.LanguagePersian]: ["اجرا"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_IF,
        data: {
            [LanguageID.LanguageEnglish]: ["if"],
            [LanguageID.LanguagePersian]: ["اگر"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_ELSE,
        data: {
            [LanguageID.LanguageEnglish]: ["else"],
            [LanguageID.LanguagePersian]: ["وگرنه"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_PRINT,
        data: {
            [LanguageID.LanguageEnglish]: ["print"],
            [LanguageID.LanguagePersian]: ["چاپ", "نمایش"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_FN,
        data: {
            [LanguageID.LanguageEnglish]: ["fn"],
            [LanguageID.LanguagePersian]: ["تابع"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_EXTERN,
        data: {
            [LanguageID.LanguageEnglish]: ["extern"],
            [LanguageID.LanguagePersian]: ["ارث"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_FOR,
        data: {
            [LanguageID.LanguageEnglish]: ["for"],
            [LanguageID.LanguagePersian]: ["برای"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_WHILE,
        data: {
            [LanguageID.LanguageEnglish]: ["while"],
            [LanguageID.LanguagePersian]: ["تا"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_REPEAT,
        data: {
            [LanguageID.LanguageEnglish]: ["repeat"],
            [LanguageID.LanguagePersian]: ["تکرار"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_DO,
        data: {
            [LanguageID.LanguageEnglish]: ["do"],
            [LanguageID.LanguagePersian]: ["انجام"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_RET,
        data: {
            [LanguageID.LanguageEnglish]: ["ret"],
            [LanguageID.LanguagePersian]: ["برگشت"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_BREAK,
        data: {
            [LanguageID.LanguageEnglish]: ["break"],
            [LanguageID.LanguagePersian]: ["شکست"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_CONTINUE,
        data: {
            [LanguageID.LanguageEnglish]: ["continue"],
            [LanguageID.LanguagePersian]: ["ادامه"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_LAYOUT,
        data: {
            [LanguageID.LanguageEnglish]: ["layout"],
            [LanguageID.LanguagePersian]: ["لایه"],
        },
    },
    {
        id: TokenKeywordType.TOKEN_BLOCK_END,
        data: {
            [LanguageID.LanguageEnglish]: ["end"],
            [LanguageID.LanguagePersian]: ["تمام"],
        },
    },
    // Types
    {
        id: TokenKeywordDataType.TOKEN_DATA_TYPE_INT,
        data: {
            [LanguageID.LanguageEnglish]: ["int"],
            [LanguageID.LanguagePersian]: ["صحیح"],
        },
    },
    {
        id: TokenKeywordDataType.TOKEN_DATA_TYPE_FLOAT,
        data: {
            [LanguageID.LanguageEnglish]: ["float"],
            [LanguageID.LanguagePersian]: ["اعشار"],
        },
    },
    {
        id: TokenKeywordDataType.TOKEN_DATA_TYPE_CHAR,
        data: {
            [LanguageID.LanguageEnglish]: ["char"],
            [LanguageID.LanguagePersian]: ["حرف"],
        },
    },
    {
        id: TokenKeywordDataType.TOKEN_DATA_TYPE_STRING,
        data: {
            [LanguageID.LanguageEnglish]: ["string"],
            [LanguageID.LanguagePersian]: ["رشته"],
        },
    },
    {
        id: TokenKeywordDataType.TOKEN_DATA_TYPE_BOOL,
        data: {
            [LanguageID.LanguageEnglish]: ["bool"],
            [LanguageID.LanguagePersian]: ["درستی"],
        },
    },
    {
        id: TokenValueType.TOKEN_BOOL_TRUE,
        data: {
            [LanguageID.LanguageEnglish]: ["true"],
            [LanguageID.LanguagePersian]: ["درست"],
        },
    },
    {
        id: TokenValueType.TOKEN_BOOL_FALSE,
        data: {
            [LanguageID.LanguageEnglish]: ["false"],
            [LanguageID.LanguagePersian]: ["غلط"],
        },
    },
];
