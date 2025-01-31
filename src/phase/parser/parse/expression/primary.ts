import { Parser } from './../parser';
import { parseExpressionLiteral } from './literal';
import { parseExpressionUnaryOperator } from './unary';
import { parseExpressionParentheses } from './parentheses';
import { AstExpression } from './../ast/expression/expression';

export function parseExpressionPrimary(parser: Parser): AstExpression | undefined {
    const currentToken = parser.currentToken;
    if (!currentToken) {
        return undefined;
    }

    // Handle literals (numbers, strings, booleans)
    const literal = parseExpressionLiteral(parser);
    if (literal) {
        return literal;
    }

    // Handle parentheses `(expr)`
    const parenthesized = parseExpressionParentheses(parser);
    if (parenthesized) {
        return parenthesized;
    }

    // Handle unary operators (-a, !b)
    const unary = parseExpressionUnaryOperator(parser);
    if (unary) {
        return unary;
    }

    return undefined;
};
