import { checkError } from '../check-error';
import { lex } from '../../../phase/lexer/lex/lex';
import { Lexer } from '../../../phase/lexer/lex/lexer';
import { parse } from '../../../phase/parser/parse/parse';
import { Parser } from '../../../phase/parser/parse/parser';
import { LanguageMap } from '../../language/language';

export function processCommandRunParser(
    fileName: string | undefined,
    absoluteDirPath: string | undefined,
    source: string,
    selectedLanguage: LanguageMap,
): number {
    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);

    const parser: Parser = new Parser(lexer);
    parse(parser);
    if (! checkError(parser, undefined, undefined)) {
        return 1;
    }
    console.log(parser.stringify());

    return 0;
};
