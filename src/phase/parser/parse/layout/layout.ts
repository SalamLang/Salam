import { Parser } from '../parser';
import { AstLayout } from '../ast/layout/layout';
import { Token } from './../../../lexer/tokenizer/token';
import { AstLayoutElement } from '../ast/layout/element';
import { parserParseLayoutElement } from './layout_element';
import { TokenKeywordType } from './../../../lexer/tokenizer/type';

export function parserParseLayout(parser: Parser): AstLayout | undefined {
    // layout:
    // ^
    const token: Token = parser.currentToken;
    parser.expect(TokenKeywordType.TOKEN_LAYOUT);

    parser.expectBlockOpen();

    const element: AstLayoutElement | undefined = parserParseLayoutElement(parser, [token]);
    if (! element) {
        return undefined;
    }

    const ast: AstLayout = new AstLayout(element);
    return ast;
};
