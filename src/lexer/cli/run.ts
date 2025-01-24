import { LanguageMap } from "./language/language";
import { Lexer } from "./../lexer/lexer";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    console.log(`Processing source in ${selectedLanguage.name}:\n${source}`);
    console.log(fileName);
    console.log(absoluteDirPath);

    const lexer = new Lexer(source, selectedLanguage);
    lexer.lex();
    lexer.print();

    return 0;
};
