import { Parser } from './../parser';
import { parseExpressionBinary } from './binary';
import { parseExpressionPrimary } from './primary';
import { getExpressionPrecedence } from './precedence';
import { Token } from '../../../lexer/tokenizer/token';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { AstExpression } from './../ast/expression/expression';
import { invalidOperators } from './../../../lexer/tokenizer/operator';

export function parseExpression(parser: Parser, precedence: number = 0): AstExpression | undefined {
    let left: AstExpression | undefined = parseExpressionPrimary(parser);
    if (!left) {
        return undefined;
    }

    while (!parser.isEnd && !parser.isBlockClose()) {
        const operator: Token = parser.currentToken;
        const isOp: boolean = operator && isOperator(operator.type);
        const isInvalidOp: boolean = isOp && invalidOperators.includes(operator.type);
        if (!operator || !isOp || isInvalidOp) {
            break;
        }
        const opPrecedence: number = getExpressionPrecedence(operator);
        if (opPrecedence < precedence) {
            break;
        }

        parser.next(); // Consume operator

        left = parseExpressionBinary(parser, left, opPrecedence, operator);
    }

    return left;
};
