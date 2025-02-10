import { Parser } from '../parser';
import { parseStatement } from './statement';
import { AstStatement } from '../ast/statement/statement';
import { AstStatementDefer } from '../ast/statement/defer';
import { TokenKeywordType } from '../../../lexer/tokenizer/type';

export function parseStatementDefer(parser: Parser): AstStatementDefer | undefined {
    parser.expect(TokenKeywordType.TOKEN_DEFER);

    if (parser.has(TokenKeywordType.TOKEN_DEFER)) {
        parser.pushError("Cannot have another defer staement after defer keyword!");
        return undefined;
    }

    const statement: AstStatement | undefined = parseStatement(parser);
    if (statement === undefined) {
        parser.pushError("error: statement defer needs a statement after defer keyword");
        return undefined;
    }

    const ast: AstStatementDefer = new AstStatementDefer(statement);
    return ast;
};
