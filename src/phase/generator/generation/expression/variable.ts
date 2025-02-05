import { Generator } from './../generator';
import { generateExpression } from './expression';
import { AstExpressionVariable } from '../../../parser/parse/ast/expression/variable';
import { ExpressionPair } from '../../../parser/parse/ast/expression/expression';

export function generateExpressionVariable(generator: Generator, expr: AstExpressionVariable): ExpressionPair {
    let result: ExpressionPair = generateExpression(generator, expr.left);
    return result;
};
