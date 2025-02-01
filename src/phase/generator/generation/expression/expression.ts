import { Generator } from './../generator';
import { generateExpressionUnary } from './unary';
import { generateExpressionBinary } from './binary';
import { generateExpressionLiteral } from './literal';
import { generateExpressionVariable } from './variable';
import { AstExpression } from '../../../parser/parse/ast/expression/expression';
import { AstExpressionUnary } from '../../../parser/parse/ast/expression/unary';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { AstExpressionLiteral } from '../../../parser/parse/ast/expression/literal';
import { AstExpressionVariable } from '../../../parser/parse/ast/expression/variable';

export function generateExpression(generator: Generator, expr: AstExpression): string {
    switch (expr.type) {
        case "ExpressionLiteral":
            return generateExpressionLiteral(generator, expr as AstExpressionLiteral);

        case "ExpressionBinary":
            return generateExpressionBinary(generator, expr as AstExpressionBinary);

        case "ExpressionUnary":
            return generateExpressionUnary(generator, expr as AstExpressionUnary);
        
        case "ExpressionVariable":
            return generateExpressionVariable(generator, expr as AstExpressionVariable);
    }
    return "Unkjnown expression type " + expr.type;
};
