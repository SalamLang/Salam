import { Parser } from './../parser';
import { TokenKeywordType } from './../../../lexer/tokenizer/type';

export function parserParseLayout(parser: Parser): void {
    console.log("Parse layout...");
    parser.expect(TokenKeywordType.TOKEN_LAYOUT);

    // while (parser.index < parser.lexer.tokens.length) {
    //     const token = parser.lexer.tokens[parser.index];
    //     if (token.type === TokenKeywordType.TOKEN_BLOCK_END) {
    //         break;
    //     }
    //     parser.index++;
    // }

    parser.expect(TokenKeywordType.TOKEN_BLOCK_END);
};
