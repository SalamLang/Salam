import { Parser } from './../parser';
import { parseExpressionLiteral } from './literal';
import { parseExpressionUnaryOperator } from './unary';
import { parseExpressionParentheses } from './parentheses';
import { AstExpression } from './../ast/expression/expression';

export function parseExpressionPrimary(parser: Parser): AstExpression | undefined {
    const currentToken = parser.currentToken;
    console.log("parseExpressionPrimary", currentToken?.type);
    if (!currentToken) {
        return undefined;
    }

    // Handle literals (numbers, strings, booleans)
    const literal = parseExpressionLiteral(parser);
    if (literal) {
        console.log("parseExpressionPrimary", literal);
        return literal;
    }

    // Handle parentheses `(expr)`
    const parenthesized = parseExpressionParentheses(parser);
    if (parenthesized) {
        console.log("parseExpressionPrimary", parenthesized);
        return parenthesized;
    }

    // Handle unary operators (-a, !b)
    const unary = parseExpressionUnaryOperator(parser);
    if (unary) {
        console.log("parseExpressionPrimary", unary);
        return unary;
    }

    return undefined;
};
