import { Generator } from '../generator';
import { generateExpression } from '../expression/expression';
import { generateFunctionParameter } from './function_parameter';
import { AstExpressionFunctionCall } from '../../../parser/parse/ast/function/function_call';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';

export function generateExpressionFunctionCall(generator: Generator, expr: AstExpressionFunctionCall): string {
    let result: string = "";
    let parameters_str: string = "";

    if (expr.value_type === undefined) {
        generator.pushError("Cannot handle a function call which is not detected!");
        return "";
    }

    if (expr.generated_value !== undefined) {
        generator.pushExtendedFunction(expr.generated_value, expr.value_type);
    }

    if (expr.left.type === "ExpressionBinary") {
        result += (expr.left as AstExpressionBinary).generated_value;
    } else {
        result += generateExpression(generator, expr.left);
    }

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
