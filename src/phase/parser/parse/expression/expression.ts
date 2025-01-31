import { Parser } from './../parser';
import { parseExpressionPrimary } from './primary';
import { getExpressionPrecedence } from './precedence';
import { AstExpression } from './../ast/expression/expression';
import { AstBinaryExpression } from './../ast/expression/binary';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpression(parser: Parser, precedence: number = 0): AstExpression | undefined {
    let left: AstExpression | undefined = parseExpressionPrimary(parser);
    if (!left) {
        return undefined;
    }

    while (!parser.isEnd && !parser.isBlockClose()) {
        const operator = parser.currentToken;
        if (!operator) {
            break;
        }

        const isOperator = operator && Object.values(TokenOperatorType).includes(operator.type as TokenOperatorType);
        if (!isOperator) {
            break;
        }
        if (operator.type === TokenOperatorType.TOKEN_RIGHT_PAREN || operator.type === TokenOperatorType.TOKEN_RIGHT_BRACE || operator.type === TokenOperatorType.TOKEN_RIGHT_BRACKET) {
            break;
        }

        const opPrecedence = getExpressionPrecedence(operator);
        if (opPrecedence < precedence) {
            break;
        }

        parser.next(); // Consume operator

        const right = parseExpression(parser, opPrecedence + 1);
        if (!right) {
            parser.pushError(`Expected expression after '${operator.type}'`);
            return left;
        }

        left = new AstBinaryExpression(left, operator.type, right);
    }

    return left;
};
