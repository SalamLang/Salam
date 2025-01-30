import { Parser } from '../parser';
import { AstExpression } from '../ast/expression/expression';
import { AstLayoutAttribute } from '../ast/layout/attribute';
import { parserParseLayoutAttributeValue } from './attribute_value';
import { Validator } from './../../../validator/validation/validator';
import { AstLayoutAttributeType } from '../ast/layout/attribute_type';
import { Token, arrayName2String } from '../../../lexer/tokenizer/token';
import { parserMessageRenderer } from '../../../../common/message/message';
import { ParserMessageKeys } from '../../../../common/message/parser/parser';

export function parserParseLayoutAttribute(parser: Parser, element_enduser_name: string, attribute_key_tokens: Token[]): AstLayoutAttribute | undefined {
    // attribute_key = attribute_value
    //                 ^
    if (attribute_key_tokens.length === 0) {
        parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_UNEXPECTED_END_OF_TOKENS_IN_LAYOUT_ATTRIBUTE));
    }

    const attribute_key: string = arrayName2String(attribute_key_tokens);

    const attribute_value: AstExpression | undefined = parserParseLayoutAttributeValue(parser, element_enduser_name, attribute_key);
    if (! attribute_value) {
        return undefined;
    }

    const isStyle = Validator.getElementAttributeStyle(parser.getLanguageId(), attribute_key);
    const attribute_kind: AstLayoutAttributeType = isStyle ? AstLayoutAttributeType.Style : AstLayoutAttributeType.Normal;

    const attribute: AstLayoutAttribute = new AstLayoutAttribute(element_enduser_name, attribute_key, attribute_value, attribute_kind);
    return attribute;
};
