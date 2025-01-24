import { LanguageMap } from "./language/language";
import { Lexer } from "./../lexer/lexer";
import { lex } from "./../lexer/lex";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    lexer.print();

    return 0;
};
