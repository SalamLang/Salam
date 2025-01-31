import { Parser } from './../parser';
import { parseExpression } from './expression';
import { AstExpression } from './../ast/expression/expression';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpressionParentheses(parser: Parser): AstExpression | undefined {
    const expr = parseExpression(parser);
    parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN); // Expect ')'
    return expr;
};
