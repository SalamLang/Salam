import { lex } from "../../lexer/lex/lex";
import { Lexer } from "../../lexer/lex/lexer";
import { parse } from "../../parser/parse/parse";
import { Parser } from "../../parser/parse/parser";
import { LanguageMap } from "../../parser/cli/language/language";

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    // lexer.print();

    const parser = new Parser(lexer);
    parse(parser);
    parser.print();

    return 0;
};
