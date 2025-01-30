import { Parser } from '../parser';
import { Token } from '../../../lexer/tokenizer/token';
import { AstStatementReturn } from './../ast/statement/return';

export function parseStatementReturn(parser: Parser): AstStatementReturn | undefined {
    const current_token: Token | undefined = parser.currentToken;
    console.log("parseStatementReturn: ", current_token.type);
    return undefined;
};
