import { Parser } from '../parser';
import { parserParseLayoutKey } from './layout_key';
import { AstLayoutElement } from '../ast/layout/element';
import { AstLayoutAttribute } from '../ast/layout/attribute';
import { parserParseLayoutAttribute } from './layout_attribute';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { TokenKeywordType, TokenOperatorType, TokenOtherType } from './../../../lexer/tokenizer/type';

export function parserParseLayoutElement(parser: Parser, tokens: Token[]): AstLayoutElement | undefined {
    const value: string = arrayName2String(tokens);

    const element: AstLayoutElement = new AstLayoutElement(value);

    while (parser.index < parser.lexer.tokens.length) {
        if (parser.currentToken.type === TokenKeywordType.TOKEN_BLOCK_END || parser.currentToken.type === TokenOtherType.TOKEN_EOF) {
            break;
        }
        // key = value
        // ^
        if (parser.currentToken.isKeyword) {
            const tokens: Token[] = parserParseLayoutKey(parser);
            // key = value
            //     ^
            if (parser.skip(TokenOperatorType.TOKEN_ASSIGN)) {
                // key = value
                //       ^
                const attribute: AstLayoutAttribute | undefined = parserParseLayoutAttribute(parser, value, tokens);
                if (attribute) {
                    if (! element.globalAttributes.push(attribute)) {
                        parser.pushError("Duplicate attribute '" + attribute.key + "' in layout");
                        return undefined;
                    }
                }
            }
            // element :
            //     ^
            else if (parser.skipBlockOpen()) {
                // element : 
                //       ^
                const element: AstLayoutElement | undefined = parserParseLayoutElement(parser, tokens);
                if (! element) {
                    parser.pushError("Unexpected token as element name, current token is '" + parser.currentToken.type + "'");
                    return undefined;
                }
                element.block.push(element);
            } else {
                parser.pushError("Unexpected token in layout, current token is '" + parser.currentToken.type + "'");
                return undefined;
            }
        } else {
            parser.pushError("Unexpected token in layout as attribute, current token is '" + parser.currentToken.type + "'");
            return undefined;
        }
    }

    parser.expectBlockClose();
    return element;
};
