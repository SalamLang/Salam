import { Parser } from './../parser';
import { parseExpression } from '../expression/expression';
import { AstExpression } from '../ast/expression/expression';
import { AstStatementReturn } from './../ast/statement/return';
import { TokenKeywordType } from '../../../lexer/tokenizer/type';

export function parseStatementReturn(parser: Parser): AstStatementReturn | undefined {
    parser.expect(TokenKeywordType.TOKEN_RET);
    const expression: AstExpression | undefined = parseExpression(parser);
    if (expression === undefined) {
        parser.pushError('Expected expression after return keyword');
        return undefined;
    }

    const ast: AstStatementReturn = new AstStatementReturn(expression);
    return ast;
};
