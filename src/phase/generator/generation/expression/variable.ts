import { Generator } from './../generator';
import { generateExpression } from './expression';
import { AstExpressionVariable } from '../../../parser/parse/ast/expression/variable';

export function generateExpressionVariable(generator: Generator, expr: AstExpressionVariable): string {
    let result: string = "";
    result += generateExpression(generator, expr.left);
    return result;
};
