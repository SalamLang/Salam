import { lex } from "../../lexer/lex/lex";
import { Lexer } from "../../lexer/lex/lexer";
import { LanguageMap } from "../language/language";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    lexer.print();

    return 0;
};
