import { Parser } from './../parser';
import { parserParseLayoutKey } from './layout_key';
import { AstLayoutElement } from '../ast/layout/element';
import { RuntimeElement } from './../../../runtime/element';
import { runtimeElements } from './../../../runtime/runtime';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { parserParseLayoutAttribute } from './layout_attribute';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { TokenKeywordType, TokenOperatorType, TokenOtherType } from './../../../lexer/tokenizer/type';

export function parserParseLayoutElement(parser: Parser, tokens: Token[]): AstLayoutElement | undefined {
    const value: string = arrayName2String(tokens);
    let runtimeElement: RuntimeElement | undefined = undefined;
    for (const runtimeElementItem of runtimeElements) {
        if (runtimeElementItem.getText(parser.lexer.language.id)?.includes(value)) {
            runtimeElement = runtimeElementItem;
            break;
        }
    }

    if (runtimeElement === undefined) {
        parser.error("Element '" + value + "' is not a valid element");
        return undefined;
    }

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
                // key = value
                //       ^
                const attribute: AstLayoutAttribute | undefined = parserParseLayoutAttribute(parser, value, runtimeElement, tokens);
                if (attribute) {
                    if (! ast.globalAttributes.push(attribute)) {
                        parser.error("Duplicate attribute '" + attribute.key + "' in layout");
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
