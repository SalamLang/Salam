import { processCommandRunLexer } from './lexer';
import { processCommandRunParser } from './parser';
import { LanguageMap } from './../../language/language';
import { processCommandRunValidator } from './validator';
import { processCommandRunGenerator } from './generator';

export const code_flag: string = "code";

export function processCommandRun(type: string, fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
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
