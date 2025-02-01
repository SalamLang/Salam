import { Generator } from './../generator';
import { AstExpressionLiteral } from '../../../parser/parse/ast/expression/literal';

export function generateExpressionLiteral(generator: Generator, expr: AstExpressionLiteral): string {
    let result: string = "";
    result += expr.getString();
    return result;
};
