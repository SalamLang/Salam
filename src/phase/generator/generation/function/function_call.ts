import { Generator } from '../generator';
import { generateExpression } from '../expression/expression';
import { generateFunctionParameters } from '../statement/function_parameters';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { AstExpressionFunctionCall } from '../../../parser/parse/ast/function/function_call';

export function generateExpressionFunctionCall(generator: Generator, expr: AstExpressionFunctionCall): ExpressionPair {
    let result: ExpressionPair = { key: '', value: '' };

    if (expr.value_type === undefined) {
        generator.pushError("Cannot handle a function call which is not detected!");
        return result;
    }

    if (expr.left.type === "ExpressionBinary") {
        result.value += (expr.left as AstExpressionBinary).getString();
    } else {
        const expr_pair: ExpressionPair = generateExpression(generator, expr.left);
        result.key += expr_pair.key;
        result.value += expr_pair.value;
    }

    result.value += "(";

    const parameters: ExpressionPair = generateFunctionParameters(generator, expr.parameters);
    result.key += parameters.key;
    result.value += parameters.value;

    result.value += ")";

    return result;
};
