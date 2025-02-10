import { Parser } from './../parser';
import { parseExpression } from './expression';
import { AstExpression } from './../ast/expression/expression';
import { AstExpressionTernary } from '../ast/expression/ternary';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpressionTernary(parser: Parser, result: AstExpression): AstExpression | undefined {
    parser.expect(TokenOperatorType.TOKEN_QUESTION);

    const true_path: AstExpression | undefined = parseExpression(parser);
    if (true_path === undefined) {
        return undefined;
    }

    parser.expect(TokenOperatorType.TOKEN_COLON);

    const false_path: AstExpression | undefined = parseExpression(parser);
    if (false_path === undefined) {
        return undefined;
    }

    const ast: AstExpressionTernary = new AstExpressionTernary(result, true_path, false_path);
    return ast;
};
