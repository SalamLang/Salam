import { lex } from "../../phase/lexer/lex/lex";
import { LanguageMap } from "../language/language";
import { Lexer } from "../../phase/lexer/lex/lexer";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    lexer.print();

    return 0;
};
