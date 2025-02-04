import { Generator } from '../generator';
import { generateExpression } from '../expression/expression';
import { AstFunctionParameter } from '../../../parser/parse/ast/function/function_parameter';

export function generateFunctionParameters(generator: Generator, parameters: AstFunctionParameter[]): string {
    let result: string = "";
    for (const parameter of parameters) {
        if (result.length > 0) {
            result += ", ";
        }

        if (parameter.value.value_type !== undefined) {
            result += generateExpression(generator, parameter.value);
        }
    }
    return result;
};
