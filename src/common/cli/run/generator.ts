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
    let fs: any = undefined;
    let path: any = undefined;
    if (typeof window === "undefined") {
        let requireFunc: any = undefined;
        try {
            requireFunc = typeof require !== "undefined" ? require : undefined;
        } catch (error) {
          console.error("Error: Unable to obtain the require function.");
          return 1;
        }
        if (requireFunc === undefined) {
            return 1;
        }
    
        try {
          fs = requireFunc("fs");
          path = requireFunc("path");
        } catch (error) {
          console.error("Error: Unable to load 'fs' or 'path' modules.");
          return 1;
        }
    }

    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);

    const parser: Parser = new Parser(lexer);
    parse(parser);
    if (typeof window !== "undefined") {
        (window as any).lexer = lexer;
        (window as any).parser = parser;
    }
    if (! checkError(parser, undefined, undefined)) {
        return 1;
    }

    const validator: Validator = new Validator(parser.ast);
    validate(validator);
    if (typeof window !== "undefined") {
        (window as any).validator = validator;
    }
    if (! checkError(parser, validator, undefined)) {
        return 1;
    }
    if (typeof window === "undefined" && fs !== undefined) {
        const astFileName: string = 'test.json';
        if (fs.existsSync(astFileName)) {
            fs.unlinkSync(astFileName);
        }
        validator.writeToFile(astFileName);
    }

    const generator: Generator = new Generator(validator.ast, validator.extendedFunctions, validator.extendedVariables, validator.packages);
    generate(generator);
    if (typeof window !== "undefined") {
        (window as any).generator = generator;
    }
    if (! checkError(parser, validator, generator)) {
        return 1;
    }

    if (typeof window === "undefined") {
        const outputFileName: string = 'test.c';
        if (fs.existsSync(outputFileName)) {
            fs.unlinkSync(outputFileName);
        }
        generator.writeToFile(outputFileName);
    }

    return 0;
};
