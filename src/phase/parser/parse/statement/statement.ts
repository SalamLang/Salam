import { Parser } from './../parser';
import { AstBlock } from './../ast/block';
import { Token } from './../../../lexer/tokenizer/token';
import { parseStatementPrint } from './print';
import { parseStatementReturn } from './return';

export function parseStatement(parser: Parser): AstBlock | undefined {
    const current_token: Token | undefined = parser.currentToken;
    console.log("parseStatement: ", current_token.type);
    switch (current_token.type) {
        case "RET": return parseStatementReturn(parser);
        case "PRINT": return parseStatementPrint(parser);
    }
    return undefined;
};
