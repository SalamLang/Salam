"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.showAvailableLanguages = showAvailableLanguages;
exports.showUsage = showUsage;
exports.processCommandHelp = processCommandHelp;
const language_1 = require("./../language");
const error_1 = require("./../error");
const data_1 = require("./../language/data");
function showAvailableLanguages() {
    console.log("Available languages:");
    data_1.languageMapsValues.forEach((lang) => console.log(`  ${lang.flag} - ${lang.name}`));
}
;
function showUsage() {
    console.log("Usage: ./salam [options] <command>");
    console.log("");
    console.log("Commands:");
    console.log("  version         Display the version of Salam CLI.");
    console.log("  help            Display this help message.");
    console.log("  file <path>     Process a file with the specified path.");
    console.log("  code <value>    Process the provided code snippet.");
    console.log("");
    console.log("Options:");
    console.log(`  ${language_1.lang_flag} <value>    Select the language to use.`);
    showAvailableLanguages();
}
;
function processCommandHelp(args) {
    if ((0, error_1.hasExtraArguments)(args)) {
        return 1;
    }
    showUsage();
    return 0;
}
;
