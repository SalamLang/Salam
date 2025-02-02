import { Generator } from '../generator';
import { generateExpression } from '../expression/expression';
import { generateFunctionParameter } from './function_parameter';
import { AstExpressionFunctionCall } from '../../../parser/parse/ast/function/function_call';

export function generateExpressionFunctionCall(generator: Generator, expr: AstExpressionFunctionCall): string {
    let result: string = "";
    let parameters_str: string = "";
    result += generateExpression(generator, expr.left);
    result += "(";

    for (const parameter of expr.parameters) {
        if (parameters_str.length > 0) {
            parameters_str += ", ";
        }
        parameters_str += generateFunctionParameter(generator, parameter);
    }

    result += parameters_str;
    result += ")";
    return result;
};
