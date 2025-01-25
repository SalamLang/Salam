"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.lang_flag = void 0;
exports.processLanguageFlag = processLanguageFlag;
const data_1 = require("./language/data");
const utils_1 = require("./language/utils");
const help_1 = require("./command/help");
exports.lang_flag = "--lang";
function processLanguageFlag(args, selectedLanguage) {
    const langFlagIndex = args.findIndex((arg) => arg === exports.lang_flag);
    if (langFlagIndex !== -1) {
        const langValue = args[langFlagIndex + 1];
        if (!langValue) {
            console.error(`Error: Missing value for ${exports.lang_flag} flag.`);
            (0, help_1.showAvailableLanguages)();
            return 1;
        }
        const languageId = (0, utils_1.getLanguageIDByFlag)(langValue);
        if (languageId !== undefined) {
            const language = data_1.languageMaps[languageId];
            Object.assign(selectedLanguage, language);
        }
        else {
            console.error(`Error: Invalid language value "${langValue}".`);
            (0, help_1.showAvailableLanguages)();
            return 1;
        }
    }
    return 0;
}
;
