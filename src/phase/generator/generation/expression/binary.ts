import { Generator } from './../generator';
import { generateExpression } from './expression';
import { generateExpressionOperator } from './operator';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';

export function generateExpressionBinary(generator: Generator, expr: AstExpressionBinary): string {
    if (expr.generated_value !== undefined) {
        return expr.generated_value;
    }

    const left: string = generateExpression(generator, expr.left);
    const right: string = generateExpression(generator, expr.right);
    const result: string = generateExpressionOperator(generator, left, expr.operator, right);
    return result;
};
