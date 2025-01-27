import { lex } from './../../phase/lexer/lex/lex';
import { LanguageMap } from './../language/language';
import { Lexer } from './../../phase/lexer/lex/lexer';
import { parse } from './../../phase/parser/parse/parse';
import { Parser } from './../../phase/parser/parse/parser';
import { validate } from './../../phase/validator/validation/validate';
import { Validator } from './../../phase/validator/validation/validator';

export function processCommandRun(fileName: string | undefined, absoluteDirPath: string | undefined, source: string, selectedLanguage: LanguageMap): number {
    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    lexer.print();

    console.log('=======================');
    console.log('=======================');
    console.log('=======================');

    const parser: Parser = new Parser(lexer);
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

    const validator: Validator = new Validator(parser.ast);
    validate(validator);
    if (validator.errors.length > 0) {
        validator.errors.forEach((error: string) => {
            console.error(error);
        });
        return 1;
    }

    return 0;
};
