"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.languageMapsValues = exports.languageMaps = void 0;
const language_1 = require("./language");
exports.languageMaps = {
    [language_1.LanguageID.LANGUAGE_ENGLISH]: {
        id: language_1.LanguageID.LANGUAGE_ENGLISH,
        name: "English",
        local_name: "English",
        flag: "en",
        keywords: [
            "if",
            "else",
            "while"
        ],
    },
    [language_1.LanguageID.LANGUAGE_PERSIAN]: {
        id: language_1.LanguageID.LANGUAGE_PERSIAN,
        name: "Persian",
        local_name: "فارسی",
        flag: "fa",
        keywords: [
            "اگر",
            "درغیراینصورت",
            "وقتی"
        ],
    }
};
exports.languageMapsValues = Object.values(exports.languageMaps);
