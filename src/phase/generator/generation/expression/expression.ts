import { Generator } from './../generator';
import { generateExpressionUnary } from './unary';
import { generateExpressionBinary } from './binary';
import { generateExpressionLiteral } from './literal';
import { generateExpressionVariable } from './variable';
import { TokenOperatorType, TokenType } from '../../../lexer/tokenizer/type';
import { AstExpression } from '../../../parser/parse/ast/expression/expression';
import { AstExpressionUnary } from '../../../parser/parse/ast/expression/unary';
import { AstExpressionBinary } from '../../../parser/parse/ast/expression/binary';
import { AstExpressionLiteral } from '../../../parser/parse/ast/expression/literal';
import { AstExpressionVariable } from '../../../parser/parse/ast/expression/variable';

export function generateExpressionOperator(generator: Generator, left: string, op: TokenType, right: string | undefined = undefined): string {
    switch (op) {
        case TokenOperatorType.TOKEN_PLUS:
            if (right === undefined) {
                return left;
            }
            return `${left} + ${right}`;
        case TokenOperatorType.TOKEN_MINUS:
            if (right === undefined) {
                return `-${left}`;
            }
            return `${left} - ${right}`;
        case TokenOperatorType.TOKEN_MULTIPLY:
            if (right === undefined) {
                return left;
            }
            return `${left} * ${right}`;
        case TokenOperatorType.TOKEN_DIVIDE:
            if (right === undefined) {
                return left;
            }
            return `${left} / ${right}`;
        case TokenOperatorType.TOKEN_MODULO:
            if (right === undefined) {
                return left;
            }
            return `${left} % ${right}`;
        case TokenOperatorType.TOKEN_POWER:
            if (right === undefined) {
                return left;
            }
            return `pow(${left}, ${right})`;
        case TokenOperatorType.TOKEN_MEMBER:
            if (right === undefined) {
                return left;
            }
            return `${left}.${right}`;
    }
    return "Unknown operator " + op;
};

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
