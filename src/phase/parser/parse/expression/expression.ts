import { Parser } from './../parser';
import { AstExpression } from './../ast/expression';
import { Token } from './../../../lexer/tokenizer/token';

export function parseExpression(parser: Parser): AstExpression | undefined {
    const current_token: Token | undefined = parser.currentToken;
    console.log("parseExpression: ", current_token.type);
    return undefined;
};
