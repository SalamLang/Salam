import { Parser } from './../parser';
import { parseExpression } from '../expression/expression';
import { AstExpression } from '../ast/expression/expression';
import { AstStatementExpression } from '../ast/statement/expression';

export function parseStatementExpression(parser: Parser): AstStatementExpression | undefined {
    const expression: AstExpression | undefined = parseExpression(parser);
    if (expression === undefined) {
        return undefined;
    }
    const ast: AstStatementExpression = new AstStatementExpression(expression);
    return ast;
};
