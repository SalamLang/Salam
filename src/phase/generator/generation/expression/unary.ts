import { Generator } from './../generator';
import { generateExpression } from './expression';
import { generateExpressionOperator } from './operator';
import { AstExpressionUnary } from '../../../parser/parse/ast/expression/unary';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';

export function generateExpressionUnary(generator: Generator, expr: AstExpressionUnary): ExpressionPair {
    const result: ExpressionPair = { key: '', value: '' };

    const left: ExpressionPair = generateExpression(generator, expr.right);
    result.key += left.key;

    const op: string = generateExpressionOperator(generator, left.value, expr.operator, undefined);
    result.value += op;

    return result;
};
