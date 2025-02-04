import { Parser } from './../parser';
import { prefix_bp_lookup } from './expression';
import { parseExpressionPrefix } from './prefix';
import { parseExpressionLiteral } from './literal';
import { Token } from '../../../lexer/tokenizer/token';
import { parseExpressionParentheses } from './parenthese';
import { AstExpression } from './../ast/expression/expression';
import { isOperator } from '../../../lexer/tokenizer/operator';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { invalidOperators } from './../../../lexer/tokenizer/operator';

export function parseExpressionPrimary(parser: Parser): AstExpression | undefined {
    console.log("parseExpressionPrimary", parser.currentToken);

    const currentToken: Token = parser.currentToken;
    const isOp: boolean = currentToken && isOperator(currentToken.type);
    const isInvalidOp: boolean = isOp && invalidOperators.includes(currentToken.type);
    // console.log(currentToken, isOp, isInvalidOp, parser.currentToken);

    if (!currentToken || isInvalidOp) {
        parser.pushError("Invalid operator inside primary expression " + currentToken.enduser_token);
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
        return parseExpressionPrefix(parser, prefix_bp_lookup(currentToken.type));
    }
    else {
        parser.pushError("Failed to parse primary expression???");
    }
    return undefined;
};
