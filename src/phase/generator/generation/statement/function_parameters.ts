import { Generator } from '../generator';
import { generateFunctionParameter } from '../function/function_parameter';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';
import { AstFunctionParameter } from '../../../parser/parse/ast/function/function_parameter';

export function generateFunctionParameters(generator: Generator, parameters: AstFunctionParameter[]): ExpressionPair {
    let result: ExpressionPair = { key: '', value: '' };
    for (const parameter of parameters) {
        if (result.value.length > 0) {
            result.value += ", ";
        }
        if (parameter.value.value_type !== undefined) {
            const parameter_pair: ExpressionPair = generateFunctionParameter(generator, parameter);
            result.key += parameter_pair.key;
            result.value += parameter_pair.value;
        }
    }
    return result;
};
