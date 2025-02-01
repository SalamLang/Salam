import { Generator } from './../generator';
import { generateExpression } from './expression';
import { AstExpressionUnary } from '../../../parser/parse/ast/expression/unary';

export function generateExpressionUnary(generator: Generator, expr: AstExpressionUnary): string {
    let result: string = "";
    result += expr.operator;
    result += " ";
    result += generateExpression(generator, expr.right);
    return result;
};
