import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { AstLayoutAttributeType } from './../ast/layout/attribute_type';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { parserParseLayoutAttributeValue } from './layout_attribute_value';
import { parserMessages } from './../../../../common/message/parser/parser';
import { messageRenderer, MessageKeys } from './../../../../common/message/message';

export function parserParseLayoutAttribute(parser: Parser, element_enduser_name: string, attribute_key_tokens: Token[]): AstLayoutAttribute | undefined {
    // attribute_key = attribute_key
    //      ^
    if (attribute_key_tokens.length === 0) {
        parser.pushError(messageRenderer(parserMessages, parser.lexer.language.id, MessageKeys.PARSER_UNEXPECTED_END_OF_TOKENS_IN_LAYOUT_ATTRIBUTE));
    }

    const attribute_key: string = arrayName2String(attribute_key_tokens);

    const attribute_value: AstExpression | undefined = parserParseLayoutAttributeValue(parser, element_enduser_name, attribute_key);
    if (! attribute_value) {
        return undefined;
    }

    const attribute: AstLayoutAttribute = new AstLayoutAttribute(element_enduser_name, attribute_key, attribute_value, AstLayoutAttributeType.Normal);
    return attribute;
};
