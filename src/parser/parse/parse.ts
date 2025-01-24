import { Parser } from './parser';
import { TokenKeywordType } from './../../lexer/tokenizer/type';
import { parserParseLayout } from './layout/layout';

export function parse(parser: Parser): void {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];

        if (token.type === TokenKeywordType.TOKEN_LAYOUT) {
            parserParseLayout(parser);
        } else {
            console.log("Unknown token type.");
        }
        parser.index++;
    }

    console.log(parser.index);
};
