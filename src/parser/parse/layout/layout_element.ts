import { Parser } from './../parser';
import { parserParseLayoutKey } from './layout_key';
import { AstLayoutElement } from '../ast/layout/element';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { parserParseLayoutAttribute } from './layout_attribute';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { TokenKeywordType, TokenOperatorType, TokenOtherType } from './../../../lexer/tokenizer/type';

export function parserParseLayoutElement(parser: Parser, tokens: Token[]): AstLayoutElement | undefined {
    const value = arrayName2String(tokens);
    const ast: AstLayoutElement = new AstLayoutElement(value);

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
                const attribute: AstLayoutAttribute | undefined = parserParseLayoutAttribute(parser, tokens);
                if (attribute) {
                    if (! ast.globalAttributes.push(attribute)) {
                        parser.error("Duplicate attribute '" + attribute.key + "' in layout");
                        return undefined;
                    }
                }
            }
            // key :
            //     ^
            else if (parser.skipBlockOpen()) {
                const element: AstLayoutElement | undefined = parserParseLayoutElement(parser, tokens);
                if (! element) {
                    parser.error("Unexpected token as element name, current token is '" + parser.currentToken.type + "'");
                    return undefined;
                }
                ast.pushElement(element);
            } else {
                parser.error("Unexpected token in layout, current token is '" + parser.currentToken.type + "'");
                return undefined;
            }
        } else {
            parser.error("Unexpected token in layout as attribute, current token is '" + parser.currentToken.type + "'");
            return undefined;
        }
    }

    parser.expectBlockClose();
    return ast;
};
