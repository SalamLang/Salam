import { Parser } from '../parser';
import { parserParseLayoutKey } from './layout_key';
import { AstLayoutElement } from '../ast/layout/element';
import { AstLayoutAttribute } from '../ast/layout/attribute';
import { parserParseLayoutAttribute } from './layout_attribute';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { TokenKeywordType, TokenOperatorType, TokenOtherType } from './../../../lexer/tokenizer/type';

export function parserParseLayoutElement(parser: Parser, element_key_tokens: Token[]): AstLayoutElement | undefined {
    const element_key: string = arrayName2String(element_key_tokens);

    const element: AstLayoutElement = new AstLayoutElement(element_key);

    while (parser.index < parser.lexer.tokens.length) {
        if (parser.currentToken.type === TokenKeywordType.TOKEN_BLOCK_END || parser.currentToken.type === TokenOtherType.TOKEN_EOF) {
            break;
        }
        // (attribute_key or element_key) (: or =)
        // ^
        if (parser.currentToken.isKeyword) {
            const key_tokens: Token[] = parserParseLayoutKey(parser);
            // (attribute_key or element_key) (: or =)
            //                                ^
            if (parser.skip(TokenOperatorType.TOKEN_ASSIGN)) {
                // attribute_key = value
                //               ^
                const attribute: AstLayoutAttribute | undefined = parserParseLayoutAttribute(parser, element_key, key_tokens);
                if (attribute) {
                    if (! element.globalAttributes.push(attribute)) {
                        parser.pushError("Duplicate attribute '" + attribute.key + "' in layout");
                        return undefined;
                    }
                }
            }
            // element_key :
            //             ^
            else if (parser.skipBlockOpen()) {
                // element_key : 
                //              ^
                const sub_element: AstLayoutElement | undefined = parserParseLayoutElement(parser, key_tokens);
                if (! sub_element) {
                    parser.pushError("Unexpected token as element name, current token is '" + parser.currentToken.type + "'");
                    return undefined;
                }
                element.block.push(sub_element);
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
