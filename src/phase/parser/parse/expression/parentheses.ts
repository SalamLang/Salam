import { Parser } from './../parser';
import { parseExpression } from './expression';
import { AstExpression } from './../ast/expression/expression';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpressionParentheses(parser: Parser): AstExpression | undefined {
    if (parser.skip(TokenOperatorType.TOKEN_LEFT_PAREN)) {
        const expr: AstExpression | undefined = parseExpression(parser); // Parse the expression inside parentheses
        parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN); // Expect ')'
        return expr;
    }
    return undefined;
};
