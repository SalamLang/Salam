import { Parser } from './../parser';
import { Token } from './../../../lexer/tokenizer/token';

export function parserParseLayoutKey(parser: Parser): Token[] {
    const token: Token = parser.currentToken;
    const key_tokens: Token[] = [];

    if (token.isKeyword) {
        key_tokens.push(token);
        
        parser.next();
        // Eating next part of the key and appending it to the tokens array
        if (!parser.isEnd) {
            while (true) {
                const ntoken = parser.currentToken;
                if (!ntoken.isKeyword || parser.isEnd) {
                    break;
                }
                key_tokens.push(ntoken);
                parser.next();
            };
        }
    }
    
    return key_tokens;
};
