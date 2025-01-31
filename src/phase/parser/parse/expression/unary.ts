import { Parser } from './../parser';
import { parseExpressionPrimary } from './primary';
import { AstUnaryExpression } from './../ast/expression/unary';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { inValidOperators } from '../../../lexer/tokenizer/operator';

export function parseExpressionUnaryOperator(parser: Parser): AstUnaryExpression | undefined {
    const currentToken = parser.currentToken;
    const isOp: boolean = isOperator(currentToken.type);
    const isInvalidOperators = inValidOperators.includes(currentToken.type);
    if (currentToken && isOp && !isInvalidOperators) {
        const operator = currentToken;
        parser.next(); // Consume unary operator

        const operand = parseExpressionPrimary(parser);
        if (!operand) {
            parser.pushError(`Expected expression after unary operator '${operator.type}'`);
            return undefined;
        }

        return new AstUnaryExpression(operator.type, operand);
    }
    return undefined;
};
