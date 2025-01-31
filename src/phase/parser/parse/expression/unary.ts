import { Parser } from './../parser';
import { parseExpressionPrimary } from './primary';
import { Token } from '../../../lexer/tokenizer/token';
import { AstExpression } from './../ast/expression/expression';
import { AstExpressionUnary } from './../ast/expression/unary';

export function parseExpressionUnary(parser: Parser): AstExpression | undefined {
    const operator: Token = parser.currentToken;
    parser.next(); // Consume unary operator

    const operand = parseExpressionPrimary(parser);
    if (!operand) {
        parser.pushError(`Expected expression after unary operator '${operator.type}'`);
        return undefined;
    }

    return new AstExpressionUnary(operator.type, operand);
};
