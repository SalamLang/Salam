import { Generator } from './../generator';
import { generateExpression, generateExpressionOperator } from './expression';
import { AstExpressionUnary } from '../../../parser/parse/ast/expression/unary';

export function generateExpressionUnary(generator: Generator, expr: AstExpressionUnary): string {
    const left: string = generateExpression(generator, expr.right);
    const result: string = generateExpressionOperator(generator, left, expr.operator, undefined);
    return result;
};
