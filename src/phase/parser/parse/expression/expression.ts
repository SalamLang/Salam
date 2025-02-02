import { Parser } from './../parser';
import { parseExpressionBinary } from './binary';
import { parseExpressionPrimary } from './primary';
import { getExpressionPrecedence } from './precedence';
import { Token } from '../../../lexer/tokenizer/token';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { AstExpression } from './../ast/expression/expression';
import { invalidOperators } from './../../../lexer/tokenizer/operator';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { parseExpressionFunctionCall } from '../function/function_call';

export function parseExpression(parser: Parser, precedence: number = 0): AstExpression | undefined {
    let left: AstExpression | undefined = parseExpressionPrimary(parser);
    if (left === undefined) {
        parser.pushError('Failed to parse primary expression');
        return undefined;
    }

    while (!parser.isEnd && !parser.isBlockClose()) {
        console.log("Left is:", left);
        const operator: Token = parser.currentToken;
        const isOp: boolean = operator && isOperator(operator.type);
        const isInvalidOp: boolean = isOp && invalidOperators.includes(operator.type);
        if (!operator || !isOp || isInvalidOp) {
            break;
        }

        // Check if this is a function call (identifier followed by `(`)
        if (parser.has(TokenOperatorType.TOKEN_LEFT_PAREN)) {
            left = parseExpressionFunctionCall(parser, left);
            if (left === undefined) {
                parser.pushError('Failed to parse function call');
                return undefined;
            }
            continue;
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
