import { checkError } from './check-error';
import { lex } from './../../phase/lexer/lex/lex';
import { LanguageMap } from './../language/language';
import { Lexer } from './../../phase/lexer/lex/lexer';
import { parse } from './../../phase/parser/parse/parse';
import { Parser } from './../../phase/parser/parse/parser';
import { validate } from './../../phase/validator/validation/validate';
import { generate } from './../../phase/generator/generation/generate';
import { Generator } from './../../phase/generator/generation/generator';
import { Validator } from './../../phase/validator/validation/validator';

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
    // parser.print();

    // console.log('=======================');
    // console.log('=======================');
    // console.log('=======================');

    const validator: Validator = new Validator(parser.ast);
    validate(validator);
    checkError(parser, validator, undefined);

    // console.log('=======================');
    // console.log('=======================');
    // console.log('=======================');

    console.log(validator.ast.stringify());

    console.log('=======================');
    console.log('=======================');
    console.log('=======================');

    const generator: Generator = new Generator(validator.ast);
    generate(generator);
    checkError(parser, validator, generator);
    generator.print();

    const outputFileName: string = 'test.html';
    generator.writeToFile(outputFileName);

    return 0;
};
