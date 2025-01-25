import { Parser } from './../parser';
import { AstNode } from './../ast/node';
import { AstLayout } from './../ast/layout/layout';
import { parserParseLayoutKey } from './layout_key';
import { Token } from './../../../lexer/tokenizer/token';
import { parserParseLayoutElement } from './layout_element';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { parserParseLayoutAttribute } from './layout_attribute';
import { TokenKeywordType, TokenOperatorType, TokenOtherType } from './../../../lexer/tokenizer/type';
import { AstLayoutElement } from '../ast/layout/element';

export function parserParseLayout(parser: Parser): AstLayout | undefined {
    const ast: AstLayout = new AstLayout();

    console.log("Parse layout...");
    parser.expect(TokenKeywordType.TOKEN_LAYOUT);
    parser.expectBlockOpen();

    while (parser.index < parser.lexer.tokens.length) {
        if (parser.currentToken.type === TokenKeywordType.TOKEN_BLOCK_END || parser.currentToken.type === TokenOtherType.TOKEN_EOF) {
            break;
        }

        // key = value
        // ^
        if (parser.currentToken.isKeyword) {
            const tokens: Token[] = parserParseLayoutKey(parser);
            console.log(tokens);

            // key = value
            //     ^
            if (parser.skip(TokenOperatorType.TOKEN_ASSIGN)) {
                const attribute: AstLayoutAttribute | undefined = parserParseLayoutAttribute(parser, tokens);
                if (attribute) {
                    if (! ast.globalAttributes.push(attribute)) {
                        parser.error("Duplicate attribute '" + attribute.key + "' in layout");
                    }
                }
            }
            // key :
            //     ^
            else if (parser.currentToken.type === TokenOperatorType.TOKEN_COLON) {
                const element: AstLayoutElement | undefined = parserParseLayoutElement(parser, tokens);
                if (element) {
                    ast.pushElement(element);
                }
            } else {
                parser.error("Unexpected token in layout, current token is '" + parser.currentToken.type + "'");
            }
        } else {
            parser.error("Unexpected token in layout as parameter, current token is '" + parser.currentToken.type + "'");
        }
    }

    parser.expectBlockClose();

    return ast;
};
