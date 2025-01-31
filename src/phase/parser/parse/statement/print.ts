import { Parser } from './../parser';
import { parseExpression } from '../expression/expression';
import { AstStatementPrint } from './../ast/statement/print';
import { AstExpression } from '../ast/expression/expression';
import { TokenKeywordType } from '../../../lexer/tokenizer/type';

export function parseStatementPrint(parser: Parser): AstStatementPrint | undefined {
    parser.expect(TokenKeywordType.TOKEN_PRINT);
    const expression: AstExpression | undefined = parseExpression(parser);
    if (expression === undefined) {
        parser.pushError('Expected expression after print keyword');
        return undefined;
    }

    const ast: AstStatementPrint = new AstStatementPrint(expression);
    return ast;
};
