import { LanguageID } from "./../cli/language/language"; 
import { TokenKeywordType } from "./../tokenizer/type";

export interface KeywordMap {
    id: TokenKeywordType;
    data?: Record<LanguageID, string[]>;
};

export const keywordMaps: Record<TokenKeywordType, KeywordMap> = {
    [TokenKeywordType.TOKEN_KEYWORD_IDENTIFIER]: {
        id: TokenKeywordType.TOKEN_KEYWORD_IDENTIFIER,
    },
    [TokenKeywordType.TOKEN_KEYWORD_IF]: {
        id: TokenKeywordType.TOKEN_KEYWORD_IF,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["if"],
            [LanguageID.LANGUAGE_PERSIAN]: ["اگر"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_ELSE]: {
        id: TokenKeywordType.TOKEN_KEYWORD_ELSE,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["else"],
            [LanguageID.LANGUAGE_PERSIAN]: ["وگرنه"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_PRINT]: {
        id: TokenKeywordType.TOKEN_KEYWORD_PRINT,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["print"],
            [LanguageID.LANGUAGE_PERSIAN]: ["چاپ"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_FN]: {
        id: TokenKeywordType.TOKEN_KEYWORD_FN,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["fn"],
            [LanguageID.LANGUAGE_PERSIAN]: ["تابع"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_FOR]: {
        id: TokenKeywordType.TOKEN_KEYWORD_FOR,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["for"],
            [LanguageID.LANGUAGE_PERSIAN]: ["برای"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_WHILE]: {
        id: TokenKeywordType.TOKEN_KEYWORD_WHILE,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["while"],
            [LanguageID.LANGUAGE_PERSIAN]: ["تا"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_REPEAT]: {
        id: TokenKeywordType.TOKEN_KEYWORD_REPEAT,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["repeat"],
            [LanguageID.LANGUAGE_PERSIAN]: ["تکرار"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_DO]: {
        id: TokenKeywordType.TOKEN_KEYWORD_DO,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["do"],
            [LanguageID.LANGUAGE_PERSIAN]: ["انجام"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_RET]: {
        id: TokenKeywordType.TOKEN_KEYWORD_RET,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["ret"],
            [LanguageID.LANGUAGE_PERSIAN]: ["برگشت"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_BREAK]: {
        id: TokenKeywordType.TOKEN_KEYWORD_BREAK,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["break"],
            [LanguageID.LANGUAGE_PERSIAN]: ["شکست"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_CONTINUE]: {
        id: TokenKeywordType.TOKEN_KEYWORD_CONTINUE,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["continue"],
            [LanguageID.LANGUAGE_PERSIAN]: ["ادامه"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_LAYOUT]: {
        id: TokenKeywordType.TOKEN_KEYWORD_LAYOUT,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["layout"],
            [LanguageID.LANGUAGE_PERSIAN]: ["صفحه"],
        },
    },
    [TokenKeywordType.TOKEN_KEYWORD_BLOCK_END]: {
        id: TokenKeywordType.TOKEN_KEYWORD_BLOCK_END,
        data: {
            [LanguageID.LANGUAGE_ENGLISH]: ["end"],
            [LanguageID.LANGUAGE_PERSIAN]: ["تمام"],
        },
    },
};

export const keywordMapsValues = Object.values(keywordMaps);
