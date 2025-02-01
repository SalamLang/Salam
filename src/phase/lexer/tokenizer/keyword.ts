import { LanguageID } from './../../../common/language/language';
import { TokenKeywordType, TokenValueType } from "./type";

export interface KeywordMap {
    id: TokenKeywordType;
    data?: Record<LanguageID, string[]>;
};

export const keywordMaps: Record<TokenKeywordType, KeywordMap> = {
    [TokenKeywordType.TOKEN_IDENTIFIER]: {
        id: TokenKeywordType.TOKEN_IDENTIFIER,
    },
    [TokenKeywordType.TOKEN_IF]: {
        id: TokenKeywordType.TOKEN_IF,
        data: {
            [LanguageID.LanguageEnglish]: ["if"],
            [LanguageID.LanguagePersian]: ["اگر"],
        },
    },
    [TokenKeywordType.TOKEN_ELSE]: {
        id: TokenKeywordType.TOKEN_ELSE,
        data: {
            [LanguageID.LanguageEnglish]: ["else"],
            [LanguageID.LanguagePersian]: ["وگرنه"],
        },
    },
    [TokenKeywordType.TOKEN_PRINT]: {
        id: TokenKeywordType.TOKEN_PRINT,
        data: {
            [LanguageID.LanguageEnglish]: ["print"],
            [LanguageID.LanguagePersian]: ["چاپ", "نمایش"],
        },
    },
    [TokenKeywordType.TOKEN_FN]: {
        id: TokenKeywordType.TOKEN_FN,
        data: {
            [LanguageID.LanguageEnglish]: ["fn"],
            [LanguageID.LanguagePersian]: ["تابع"],
        },
    },
    [TokenKeywordType.TOKEN_FOR]: {
        id: TokenKeywordType.TOKEN_FOR,
        data: {
            [LanguageID.LanguageEnglish]: ["for"],
            [LanguageID.LanguagePersian]: ["برای"],
        },
    },
    [TokenKeywordType.TOKEN_WHILE]: {
        id: TokenKeywordType.TOKEN_WHILE,
        data: {
            [LanguageID.LanguageEnglish]: ["while"],
            [LanguageID.LanguagePersian]: ["تا"],
        },
    },
    [TokenKeywordType.TOKEN_REPEAT]: {
        id: TokenKeywordType.TOKEN_REPEAT,
        data: {
            [LanguageID.LanguageEnglish]: ["repeat"],
            [LanguageID.LanguagePersian]: ["تکرار"],
        },
    },
    [TokenKeywordType.TOKEN_DO]: {
        id: TokenKeywordType.TOKEN_DO,
        data: {
            [LanguageID.LanguageEnglish]: ["do"],
            [LanguageID.LanguagePersian]: ["انجام"],
        },
    },
    [TokenKeywordType.TOKEN_RET]: {
        id: TokenKeywordType.TOKEN_RET,
        data: {
            [LanguageID.LanguageEnglish]: ["ret"],
            [LanguageID.LanguagePersian]: ["برگشت"],
        },
    },
    [TokenKeywordType.TOKEN_BREAK]: {
        id: TokenKeywordType.TOKEN_BREAK,
        data: {
            [LanguageID.LanguageEnglish]: ["break"],
            [LanguageID.LanguagePersian]: ["شکست"],
        },
    },
    [TokenKeywordType.TOKEN_CONTINUE]: {
        id: TokenKeywordType.TOKEN_CONTINUE,
        data: {
            [LanguageID.LanguageEnglish]: ["continue"],
            [LanguageID.LanguagePersian]: ["ادامه"],
        },
    },
    [TokenKeywordType.TOKEN_LAYOUT]: {
        id: TokenKeywordType.TOKEN_LAYOUT,
        data: {
            [LanguageID.LanguageEnglish]: ["layout"],
            [LanguageID.LanguagePersian]: ["لایه"],
        },
    },
    [TokenKeywordType.TOKEN_BLOCK_END]: {
        id: TokenKeywordType.TOKEN_BLOCK_END,
        data: {
            [LanguageID.LanguageEnglish]: ["end"],
            [LanguageID.LanguagePersian]: ["تمام"],
        },
    },
    // Types
    [TokenKeywordType.TOKEN_TYPE_INT]: {
        id: TokenKeywordType.TOKEN_TYPE_INT,
        data: {
            [LanguageID.LanguageEnglish]: ["int"],
            [LanguageID.LanguagePersian]: ["صحیح"],
        },
    },
    [TokenKeywordType.TOKEN_TYPE_FLOAT]: {
        id: TokenKeywordType.TOKEN_TYPE_FLOAT,
        data: {
            [LanguageID.LanguageEnglish]: ["float"],
            [LanguageID.LanguagePersian]: ["اعشار"],
        },
    },
    [TokenKeywordType.TOKEN_TYPE_CHAR]: {
        id: TokenKeywordType.TOKEN_TYPE_CHAR,
        data: {
            [LanguageID.LanguageEnglish]: ["char"],
            [LanguageID.LanguagePersian]: ["حرف"],
        },
    },
    [TokenKeywordType.TOKEN_TYPE_STRING]: {
        id: TokenKeywordType.TOKEN_TYPE_STRING,
        data: {
            [LanguageID.LanguageEnglish]: ["string"],
            [LanguageID.LanguagePersian]: ["رشته"],
        },
    },
    [TokenKeywordType.TOKEN_TYPE_BOOL]: {
        id: TokenKeywordType.TOKEN_TYPE_BOOL,
        data: {
            [LanguageID.LanguageEnglish]: ["bool"],
            [LanguageID.LanguagePersian]: ["درستی"],
        },
    },
    [TokenValueType.TOKEN_BOOL_TRUE]: {
        id: TokenValueType.TOKEN_BOOL_TRUE,
        data: {
            [LanguageID.LanguageEnglish]: ["true"],
            [LanguageID.LanguagePersian]: ["درست"],
        },
    },
    [TokenValueType.TOKEN_BOOL_FALSE]: {
        id: TokenValueType.TOKEN_BOOL_FALSE,
        data: {
            [LanguageID.LanguageEnglish]: ["false"],
            [LanguageID.LanguagePersian]: ["غلط"],
        },
    },
};

export const keywordMapsValues: KeywordMap[] = Object.values(keywordMaps);
