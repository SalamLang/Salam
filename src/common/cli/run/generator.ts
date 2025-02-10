import * as fs from 'fs';

import { checkError } from '../check-error';
import { lex } from '../../../phase/lexer/lex/lex';
import { LanguageMap } from '../../language/language';
import { Lexer } from '../../../phase/lexer/lex/lexer';
import { parse } from '../../../phase/parser/parse/parse';
import { Parser } from '../../../phase/parser/parse/parser';
import { validate } from '../../../phase/validator/validation/validate';
import { generate } from '../../../phase/generator/generation/generate';
import { Generator } from '../../../phase/generator/generation/generator';
import { Validator } from '../../../phase/validator/validation/validator';

export function processCommandRunGenerator(
    fileName: string | undefined,
    absoluteDirPath: string | undefined,
    source: string,
    selectedLanguage: LanguageMap,
): number {
    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);
    // console.log(lexer.tokens);

    const parser: Parser = new Parser(lexer);
    parse(parser);
    checkError(parser, undefined, undefined);

    const validator: Validator = new Validator(parser.ast);
    validate(validator);
    checkError(parser, validator, undefined);
    const astFileName: string = 'test.json';
    if (fs.existsSync(astFileName)) {
        fs.unlinkSync(astFileName);
    }
    validator.writeToFile(astFileName);

    const generator: Generator = new Generator(validator.ast, validator.extendedFunctions, validator.extendedVariables, validator.packages);
    generate(generator);
    const outputFileName: string = 'test.c';
    if (fs.existsSync(outputFileName)) {
        fs.unlinkSync(outputFileName);
    }
    generator.writeToFile(outputFileName);
    checkError(parser, validator, generator);

    return 0;
};
