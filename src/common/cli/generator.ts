import { lex } from '../../phase/lexer/lex/lex';
import { LanguageMap } from '../language/language';
import { Lexer } from '../../phase/lexer/lex/lexer';
import { parse } from '../../phase/parser/parse/parse';
import { Parser } from '../../phase/parser/parse/parser';
import { validate } from '../../phase/validator/validation/validate';
import { generate } from '../../phase/generator/generation/generate';
import { Generator } from '../../phase/generator/generation/generator';
import { Validator } from '../../phase/validator/validation/validator';

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

    const validator = new Validator(parser.ast);
    validate(validator);
    if (validator.errors.length > 0) {
        validator.errors.forEach((error: string) => {
            console.error(error);
        });
        return 1;
    }

    console.log('=======================');
    console.log('=======================');
    console.log('=======================');

    console.log(validator.ast.stringify());

    return 0;

    const generator = new Generator(validator.ast);
    generate(generator);
    if (generator.errors.length > 0) {
        generator.errors.forEach((error: string) => {
            console.error(error);
        });
        return 1;
    }
    generator.print();

    return 0;
};
