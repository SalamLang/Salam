import { Parser } from './../parser';
import { parserParseLayoutKey } from './layout_key';
import { AstLayoutElement } from './../ast/layout/element';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { parserParseLayoutAttribute } from './layout_attribute';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { parserMessages } from './../../../../common/message/parser/parser';
import { messageRenderer, ParserMessageKeys } from './../../../../common/message/message';
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
                        parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_DUPLICATE_ATTRIBUTE_IN_LAYOUT], attribute.key));
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
                    parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_AS_ELEMENT_NAME], parser.currentToken.type));
                    return undefined;
                }
                element.block.push(sub_element);
            } else {
                parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT], parser.currentToken.type));
                return undefined;
            }
        } else {
            parser.pushError(messageRenderer(parserMessages[parser.lexer.language.id][ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT_AS_ATTRIBUTE], parser.currentToken.type));
            return undefined;
        }
    }

    parser.expectBlockClose();
    return element;
};
