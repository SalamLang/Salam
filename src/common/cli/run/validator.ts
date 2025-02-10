import { checkError } from '../check-error';
import { lex } from '../../../phase/lexer/lex/lex';
import { LanguageMap } from '../../language/language';
import { Lexer } from '../../../phase/lexer/lex/lexer';
import { parse } from '../../../phase/parser/parse/parse';
import { Parser } from '../../../phase/parser/parse/parser';
import { validate } from '../../../phase/validator/validation/validate';
import { Validator } from '../../../phase/validator/validation/validator';

export function processCommandRunValidator(
    fileName: string | undefined, 
    absoluteDirPath: string | undefined, 
    source: string, 
    selectedLanguage: LanguageMap, 
): number {
    const lexer: Lexer = new Lexer(source, selectedLanguage, fileName, absoluteDirPath);
    lex(lexer);

    const parser: Parser = new Parser(lexer);
    parse(parser);
    checkError(parser, undefined, undefined);

    const validator: Validator = new Validator(parser.ast);
    validate(validator);
    checkError(parser, undefined, undefined);

    return 0;
};
