import { Parser } from '../parser';
import { parserParseLayoutKey } from './key';
import { AstLayoutElement } from '../ast/layout/element';
import { AstLayoutAttribute } from '../ast/layout/attribute';
import { parserParseLayoutAttribute } from './attribute';
import { Token, arrayName2String } from '../../../lexer/tokenizer/token';
import { parserMessageRenderer } from '../../../../common/message/message';
import { ParserMessageKeys } from '../../../../common/message/parser/parser';
import { TokenKeywordType, TokenOperatorType, TokenOtherType } from '../../../lexer/tokenizer/type';

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
                // attribute_key = attribute_value
                //               ^
                const attribute: AstLayoutAttribute | undefined = parserParseLayoutAttribute(parser, element_key, key_tokens);
                if (attribute) {
                    if (attribute.isStyle()) {
                        if (! element.styles.push(attribute)) {
                            parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_DUPLICATE_STYLE_ATTRIBUTE_IN_LAYOUT, attribute.key));
                            return undefined;
                        }
                    } else {
                        if (! element.attributes.push(attribute)) {
                            parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_DUPLICATE_ELEMENT_ATTRIBUTE_IN_LAYOUT, attribute.key));
                            return undefined;
                        }
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
                    parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_AS_ELEMENT_NAME, parser.currentToken.type));
                    return undefined;
                }
                element.block.push(sub_element);
            } else {
                parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT, parser.currentToken.type));
                return undefined;
            }
        } else {
            parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT_AS_ATTRIBUTE, parser.currentToken.type));
            return undefined;
        }
    }

    parser.expectBlockClose();
    return element;
};
