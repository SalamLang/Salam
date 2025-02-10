import { lex } from "../../../phase/lexer/lex/lex";
import { LanguageMap } from "../../language/language";
import { Lexer } from "../../../phase/lexer/lex/lexer";

export function processCommandRunLexer(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    console.log(lexer.stringify());

    return 0;
};
