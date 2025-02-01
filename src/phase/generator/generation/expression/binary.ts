import { Generator } from './../generator';
import { generateExpression, generateExpressionOperator } from './expression';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';

export function generateExpressionBinary(generator: Generator, expr: AstExpressionBinary): string {
    const left: string = generateExpression(generator, expr.left);
    const right: string = generateExpression(generator, expr.right);
    const result: string = generateExpressionOperator(generator, left, expr.operator, right);
    return result;
};
