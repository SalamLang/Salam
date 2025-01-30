import { Parser } from './../parser';
import { Token } from './../../../lexer/tokenizer/token';
import { AstStatementPrint } from './../ast/statement/print';

export function parseStatementPrint(parser: Parser): AstStatementPrint | undefined {
    const current_token: Token | undefined = parser.currentToken;
    console.log("parseStatementPrint: ", current_token.type);
    return undefined;
};
