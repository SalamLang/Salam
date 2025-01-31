import { checkError } from './check-error';
import { lex } from './../../phase/lexer/lex/lex';
import { Lexer } from './../../phase/lexer/lex/lexer';
import { parse } from './../../phase/parser/parse/parse';
import { Parser } from './../../phase/parser/parse/parser';
import { LanguageMap } from './../../common/language/language';

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    // lexer.print();

    // console.log('=======================');
    // console.log('=======================');
    // console.log('=======================');

    const parser: Parser = new Parser(lexer);
    parse(parser);
    checkError(parser, undefined, undefined);
    parser.print();

    return 0;
};
