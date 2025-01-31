import { Parser } from './../parser';
import { parseExpression } from './expression';
import { AstExpression } from './../ast/expression/expression';
import { AstUnaryExpression } from './../ast/expression/unary';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpressionPrimary(parser: Parser): AstExpression | undefined {
    const currentToken = parser.currentToken;
    if (!currentToken) {
        return undefined;
    }
    const isOperator = Object.values(TokenOperatorType).includes(currentToken.type as TokenOperatorType);

    // Handle literals (numbers, strings, booleans)
    if (currentToken.data !== undefined && !isOperator) {
        parser.next();
        return new AstExpression(currentToken.data.getValueString());
    }

    // Handle parentheses `(expr)`
    if (parser.skip(TokenOperatorType.TOKEN_LEFT_PAREN)) {
        const expr = parseExpression(parser);
        parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN); // Expect ')'
        return expr;
    }

    // Handle unary operators (-a, !b)
    if (isOperator) {
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
