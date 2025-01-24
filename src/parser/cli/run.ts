import { LanguageMap } from "./language/language";
import { Lexer } from "./../../lexer/lex/lexer";
import { lex } from "./../../lexer/lex/lex";
import { Parser } from "./../parse/parser";
import { parse } from "./../parse/parse";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    // lexer.print();

    const parser = new Parser(lexer);
    parse(parser);
    parser.print();

    return 0;
};
