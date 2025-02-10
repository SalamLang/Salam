import { Generator } from '../generator';
import { generateType } from '../expression/type';
import { AstFunctionArgument } from '../../../parser/parse/ast/function/function_argument';

export function generateFunctionArguments(generator: Generator, args: AstFunctionArgument[]): string {
    let result: string = "";
    for (const argument of args) {
        if (result.length > 0) {
            result += ", ";
        }

        if (argument.value_type !== undefined) {
            const param_type: string = generateType(generator, argument.value_type);
            result += param_type;
            result += " ";
            result += argument.name;
        }
    }
    return result;
};
