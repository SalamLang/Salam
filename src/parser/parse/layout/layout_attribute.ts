import { Parser } from './../parser';
import { AstExpression } from './../ast/expression';
import { RuntimeElement } from './../../../runtime/element';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { AstLayoutAttributeType } from './../ast/layout/attribute_type';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { parserParseLayoutAttributeValue } from './layout_attribute_value';
import { RuntimeElementAttribute } from './../../../runtime/element_attribute';

export function parserParseLayoutAttribute(parser: Parser, elementName: string, runtimeElement: RuntimeElement, tokens: Token[]): AstLayoutAttribute | undefined {
    // key = value
    //      ^
    if (tokens.length === 0) {
        parser.error("Unexpected end of tokens in layout attribute");
    }

    const key: string = arrayName2String(tokens);

    let runtimeElementAttribute: RuntimeElementAttribute | undefined = undefined;
    for (const runtimeElementAttributeItem of runtimeElement.attributes) {
        if (runtimeElementAttributeItem.getText(parser.lexer.language.id)?.includes(key)) {
            runtimeElementAttribute = runtimeElementAttributeItem;
            break;
        }
    }

    if (runtimeElementAttribute === undefined) {
        parser.error("Attribute '" + key + "' is not a valid attribute for element '" + elementName + "'");
        return undefined;
    }

    const value: AstExpression | undefined = parserParseLayoutAttributeValue(parser, key);
    if (! value) {
        return undefined;
    }

    return new AstLayoutAttribute(key, value, AstLayoutAttributeType.Normal);
};
