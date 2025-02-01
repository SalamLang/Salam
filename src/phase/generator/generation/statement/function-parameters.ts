import { Generator } from './../generator';
import { generateType } from '../expression/type';
import { AstFunctionAttribute } from '../../../parser/parse/ast/function_attribute';

export function generateFunctionParameters(generator: Generator, parameters: AstFunctionAttribute[]): string {
    let result: string = "";
    for (const parameter of parameters) {
        if (result.length > 0) {
            result += ", ";
        }

        const param_type: string = generateType(generator, parameter.value_type);
        result += param_type;
        result += " ";
        result += parameter.name;
    }
    return result;
};
