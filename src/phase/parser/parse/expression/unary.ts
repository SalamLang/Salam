import { Parser } from './../parser';
import { parseExpressionPrimary } from './primary';
import { AstUnaryExpression } from './../ast/expression/unary';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpressionUnaryOperator(parser: Parser): AstUnaryExpression | undefined {
    const currentToken = parser.currentToken;
    if (currentToken && Object.values(TokenOperatorType).includes(currentToken.type as TokenOperatorType)) {
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
