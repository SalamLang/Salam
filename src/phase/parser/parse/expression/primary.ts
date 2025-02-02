import { Parser } from './../parser';
import { parseExpressionUnary } from './unary';
import { parseExpressionLiteral } from './literal';
import { Token } from '../../../lexer/tokenizer/token';
import { parseExpressionParentheses } from './parenthese';
import { parseExpressionFunctionCall } from '../function/function_call';
import { AstExpression } from './../ast/expression/expression';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { invalidOperators } from './../../../lexer/tokenizer/operator';

export function parseExpressionPrimary(parser: Parser): AstExpression | undefined {
    const currentToken: Token = parser.currentToken;
    const isOp: boolean = currentToken && isOperator(currentToken.type);
    const isInvalidOp: boolean = isOp && invalidOperators.includes(currentToken.type);
    if (!currentToken || isInvalidOp) {
        return undefined;
    }
    // Handle parentheses `(expr)`
    else if (parser.skip(TokenOperatorType.TOKEN_LEFT_PAREN)) {
        return parseExpressionParentheses(parser);
    }
    // Handle literals (numbers, strings, booleans)
    else if (currentToken.data !== undefined && !isOp) {
        return parseExpressionLiteral(parser);
    }
    // Handle unary operators (-a, !b)
    else if (isOp && !isInvalidOp) {
        return parseExpressionUnary(parser);
    }
    return undefined;
};
