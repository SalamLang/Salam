"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const data_1 = require("./cli/language/data");
const cli_1 = require("./cli/cli");
const language_1 = require("./cli/language");
function main(args) {
    let selectedLanguage = data_1.languageMaps[0];
    const result = (0, language_1.processLanguageFlag)(args, selectedLanguage);
    if (result !== 0) {
        return result;
    }
    return (0, cli_1.processCommand)(args, selectedLanguage);
}
;
const args = process?.argv?.slice(2) || [];
process.exit(main(args));
