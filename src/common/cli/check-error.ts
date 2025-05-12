import { Parser } from './../../phase/parser/parse/parser';
import { Validator } from '../../phase/validator/validation/validator';
import { Generator } from './../../phase/generator/generation/generator';

export function checkError(parser: Parser | undefined, validator: Validator | undefined, generator: Generator | undefined): boolean {
    if (parser) {
        if (parser.ast.errors.length > 0) {
            parser.ast.errors.forEach((error: string) => {
                console.error(error);
            });
            return false;
        }
    }
    
    if (validator !== undefined) {
        if (validator.errors.length > 0) {
            validator.errors.forEach((error: string) => {
                console.error(error);
            });
            return false;
        }
    }

    if (generator !== undefined) {
        if (generator.errors.length > 0) {
            generator.errors.forEach((error: string) => {
                console.error(error);
            });
            return false;
        }
    }

    return true;
};
