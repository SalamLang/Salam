import { Parser } from './../parser';
import { TokenKeywordType } from './../../../lexer/tokenizer/type';

export function parserParseFunction(parser: Parser): void {
    console.log("Parse function...");

    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];
        if (token.type === TokenKeywordType.TOKEN_BLOCK_END) {
            break;
        }
        parser.index++;
    }
};
