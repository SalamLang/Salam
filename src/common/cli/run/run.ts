import { processCommandRunLexer } from './lexer';
import { processCommandRunParser } from './parser';
import { LanguageMap } from './../../language/language';
import { processCommandRunValidator } from './validator';
import { processCommandRunGenerator } from './generator';
import { languageMapsValues } from './../../language/data';

export const code_flag: string = "code";

export function processCommandRun(
    type: string,
    fileName: string | undefined,
    absoluteDirPath: string | undefined,
    source: string,
    selectedLanguage: LanguageMap,
    languageCheck: number
): number {
    // No language has been selected, so let's attempt to select one automatically.
    if (languageCheck === -1 && fileName !== undefined) {
        if (fileName.endsWith(".en.salam")) {
            selectedLanguage = Object.assign({}, languageMapsValues[0]);
        } else if (fileName.endsWith(".fa.salam")) {
            selectedLanguage = Object.assign({}, languageMapsValues[1]);
        }
    }

    switch (type) {
        case "lexer": return processCommandRunLexer(fileName, absoluteDirPath, source, selectedLanguage);
        case "parser": return processCommandRunParser(fileName, absoluteDirPath, source, selectedLanguage);
        case "validator": return processCommandRunValidator(fileName, absoluteDirPath, source, selectedLanguage);
        case "generator": return processCommandRunGenerator(fileName, absoluteDirPath, source, selectedLanguage);
        default: {
            console.error("Error: invalid type for running a script! '" + type + "'");
            return 1;
        }
    }
};
