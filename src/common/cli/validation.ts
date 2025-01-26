import { lex } from './../../phase/lexer/lex/lex';
import { Lexer } from './../../phase/lexer/lex/lexer';
import { parse } from './../../phase/parser/parse/parse';
import { Parser } from './../../phase/parser/parse/parser';
import { LanguageMap } from './../../common/language/language';
import { Validation } from './../../phase/validation/validator/validation';
import { validate } from './../../phase/validation/validator/validate';

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

    console.log('=======================');
    console.log('=======================');
    console.log('=======================');

    const validation = new Validation(parser.ast);
    validate(validation);
    if (validation.errors.length > 0) {
        validation.errors.forEach((error: string) => {
            console.error(error);
        });
        return 1;
    }

    // console.log('=======================');
    // console.log('=======================');
    // console.log('=======================');

    return 0;
};
