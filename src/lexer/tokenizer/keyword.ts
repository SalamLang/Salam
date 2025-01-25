import { LanguageID } from './../../common/language/language';
import { TokenKeywordType } from "./../tokenizer/type";

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
            [LanguageID.English]: ["if"],
            [LanguageID.Persian]: ["اگر"],
        },
    },
    [TokenKeywordType.TOKEN_ELSE]: {
        id: TokenKeywordType.TOKEN_ELSE,
        data: {
            [LanguageID.English]: ["else"],
            [LanguageID.Persian]: ["وگرنه"],
        },
    },
    [TokenKeywordType.TOKEN_PRINT]: {
        id: TokenKeywordType.TOKEN_PRINT,
        data: {
            [LanguageID.English]: ["print"],
            [LanguageID.Persian]: ["چاپ"],
        },
    },
    [TokenKeywordType.TOKEN_FN]: {
        id: TokenKeywordType.TOKEN_FN,
        data: {
            [LanguageID.English]: ["fn"],
            [LanguageID.Persian]: ["تابع"],
        },
    },
    [TokenKeywordType.TOKEN_FOR]: {
        id: TokenKeywordType.TOKEN_FOR,
        data: {
            [LanguageID.English]: ["for"],
            [LanguageID.Persian]: ["برای"],
        },
    },
    [TokenKeywordType.TOKEN_WHILE]: {
        id: TokenKeywordType.TOKEN_WHILE,
        data: {
            [LanguageID.English]: ["while"],
            [LanguageID.Persian]: ["تا"],
        },
    },
    [TokenKeywordType.TOKEN_REPEAT]: {
        id: TokenKeywordType.TOKEN_REPEAT,
        data: {
            [LanguageID.English]: ["repeat"],
            [LanguageID.Persian]: ["تکرار"],
        },
    },
    [TokenKeywordType.TOKEN_DO]: {
        id: TokenKeywordType.TOKEN_DO,
        data: {
            [LanguageID.English]: ["do"],
            [LanguageID.Persian]: ["انجام"],
        },
    },
    [TokenKeywordType.TOKEN_RET]: {
        id: TokenKeywordType.TOKEN_RET,
        data: {
            [LanguageID.English]: ["ret"],
            [LanguageID.Persian]: ["برگشت"],
        },
    },
    [TokenKeywordType.TOKEN_BREAK]: {
        id: TokenKeywordType.TOKEN_BREAK,
        data: {
            [LanguageID.English]: ["break"],
            [LanguageID.Persian]: ["شکست"],
        },
    },
    [TokenKeywordType.TOKEN_CONTINUE]: {
        id: TokenKeywordType.TOKEN_CONTINUE,
        data: {
            [LanguageID.English]: ["continue"],
            [LanguageID.Persian]: ["ادامه"],
        },
    },
    [TokenKeywordType.TOKEN_LAYOUT]: {
        id: TokenKeywordType.TOKEN_LAYOUT,
        data: {
            [LanguageID.English]: ["layout"],
            [LanguageID.Persian]: ["صفحه"],
        },
    },
    [TokenKeywordType.TOKEN_BLOCK_END]: {
        id: TokenKeywordType.TOKEN_BLOCK_END,
        data: {
            [LanguageID.English]: ["end"],
            [LanguageID.Persian]: ["تمام"],
        },
    },
};

export const keywordMapsValues = Object.values(keywordMaps);
