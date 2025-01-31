import { Parser } from './../parser';
import { parseStatementPrint } from './print';
import { parseStatementReturn } from './return';
import { Token } from './../../../lexer/tokenizer/token';
import { AstStatement } from './../ast/statement/statement';

export function parseStatement(parser: Parser): AstStatement | undefined {
    const current_token: Token | undefined = parser.currentToken;
    switch (current_token.type) {
        case "RET": return parseStatementReturn(parser);
        case "PRINT": return parseStatementPrint(parser);
    }
    return undefined;
};
