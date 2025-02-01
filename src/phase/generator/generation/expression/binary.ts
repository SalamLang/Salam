import { Generator } from './../generator';
import { generateExpression } from './expression';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';

export function generateExpressionBinary(generator: Generator, expr: AstExpressionBinary): string {
    let result: string = "";
    result += generateExpression(generator, expr.left);
    result += " ";
    result += expr.operator;
    result += " ";
    result += generateExpression(generator, expr.right);
    return result;
};
