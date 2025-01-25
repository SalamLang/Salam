import { Parser } from './parser';
import { TokenKeywordType, TokenOtherType } from './../../lexer/tokenizer/type';
import { parserParseLayout } from './layout/layout';
import { parserParseFunction } from './statement/function';

export function parse(parser: Parser): void {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];

        if (token.type === TokenOtherType.TOKEN_EOF) {
            break;
        } else if (token.type === TokenKeywordType.TOKEN_FN) {
            parserParseFunction(parser);
        } else if (token.type === TokenKeywordType.TOKEN_LAYOUT) {
            parserParseLayout(parser);
        } else {
            console.log("Unknown token type.");
            console.log(token);
        }
        parser.index++;
    }

    console.log(parser.index);
};
