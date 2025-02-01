import { Generator } from './../generator';
import { generateType } from '../expression/type';
import { AstFunctionParameter } from '../../../parser/parse/ast/function_parameter';

export function generateFunctionParameters(generator: Generator, parameters: AstFunctionParameter[]): string {
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
