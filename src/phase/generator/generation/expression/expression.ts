import { Generator } from './../generator';
import { generateExpressionUnary } from './unary';
import { generateExpressionBinary } from './binary';
import { generateExpressionLiteral } from './literal';
import { AstExpression } from '../../../parser/parse/ast/expression/expression';
import { AstExpressionUnary } from '../../../parser/parse/ast/expression/unary';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { AstExpressionLiteral } from '../../../parser/parse/ast/expression/literal';

export function generateExpression(generator: Generator, expr: AstExpression): string {
    switch (expr.type) {
        case "ExpressionLiteral":
            return generateExpressionLiteral(generator, expr as AstExpressionLiteral);

        case "ExpressionBinary":
            return generateExpressionBinary(generator, expr as AstExpressionBinary);

        case "ExpressionUnary":
            return generateExpressionUnary(generator, expr as AstExpressionUnary);
    }
    return "Unkjnown expression type " + expr.type;
};
