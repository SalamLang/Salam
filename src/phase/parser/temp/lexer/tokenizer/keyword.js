"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.keywordMapsValues = exports.keywordMaps = void 0;
const language_1 = require("./../cli/language/language");
const type_1 = require("./../tokenizer/type");
;
exports.keywordMaps = {
    [type_1.TokenKeywordType.TOKEN_IDENTIFIER]: {
        id: type_1.TokenKeywordType.TOKEN_IDENTIFIER,
    },
    [type_1.TokenKeywordType.TOKEN_IF]: {
        id: type_1.TokenKeywordType.TOKEN_IF,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["if"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["اگر"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_ELSE]: {
        id: type_1.TokenKeywordType.TOKEN_ELSE,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["else"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["وگرنه"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_PRINT]: {
        id: type_1.TokenKeywordType.TOKEN_PRINT,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["print"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["چاپ"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_FN]: {
        id: type_1.TokenKeywordType.TOKEN_FN,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["fn"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["تابع"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_FOR]: {
        id: type_1.TokenKeywordType.TOKEN_FOR,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["for"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["برای"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_WHILE]: {
        id: type_1.TokenKeywordType.TOKEN_WHILE,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["while"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["تا"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_REPEAT]: {
        id: type_1.TokenKeywordType.TOKEN_REPEAT,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["repeat"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["تکرار"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_DO]: {
        id: type_1.TokenKeywordType.TOKEN_DO,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["do"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["انجام"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_RET]: {
        id: type_1.TokenKeywordType.TOKEN_RET,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["ret"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["برگشت"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_BREAK]: {
        id: type_1.TokenKeywordType.TOKEN_BREAK,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["break"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["شکست"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_CONTINUE]: {
        id: type_1.TokenKeywordType.TOKEN_CONTINUE,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["continue"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["ادامه"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_LAYOUT]: {
        id: type_1.TokenKeywordType.TOKEN_LAYOUT,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["layout"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["صفحه"],
        },
    },
    [type_1.TokenKeywordType.TOKEN_BLOCK_END]: {
        id: type_1.TokenKeywordType.TOKEN_BLOCK_END,
        data: {
            [language_1.LanguageID.LANGUAGE_ENGLISH]: ["end"],
            [language_1.LanguageID.LANGUAGE_PERSIAN]: ["تمام"],
        },
    },
};
exports.keywordMapsValues = Object.values(exports.keywordMaps);
