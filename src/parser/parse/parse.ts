import { Parser } from './parser';
import { AstLayout } from './ast/layout/layout';
import { parserParseLayout } from './layout/layout';
import { parserParseFunction } from './statement/function';
import { TokenKeywordType, TokenOtherType } from './../../lexer/tokenizer/type';

export function parse(parser: Parser): void {
    while (parser.index < parser.lexer.tokens.length) {
        const token = parser.lexer.tokens[parser.index];

        if (token.type === TokenOtherType.TOKEN_EOF) {
            break;
        } else if (token.type === TokenKeywordType.TOKEN_FN) {
            parserParseFunction(parser);
        } else if (token.type === TokenKeywordType.TOKEN_LAYOUT) {
            const layout: AstLayout | undefined = parserParseLayout(parser);
            if (layout) {
                if (!parser.ast.setLayout(layout)) {
                    parser.error("Duplicate layout definition.");
                }
            }
        } else {
            console.log("Unknown token type.");
            console.log(token);
            break;
        }
    }

    console.log(parser.index);
};
