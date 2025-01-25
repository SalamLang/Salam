import { lex } from './../../lexer/lex/lex';
import { Lexer } from './../../lexer/lex/lexer';
import { parse } from './../../parser/parse/parse';
import { Parser } from './../../parser/parse/parser';
import { LanguageMap } from './../../common/language/language';

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    lexer.print();

    console.log('=======================');
    console.log('=======================');
    console.log('=======================');

    const parser = new Parser(lexer);
    parse(parser);
    if (parser.ast.errors.length > 0) {
        parser.ast.errors.forEach((error: string) => {
            console.error(error);
        });
        return 1;
    }
    parser.print();

    return 0;
};
