import { Parser } from '../parser';
import { AstExpression } from '../ast/expression/expression';
import { AstLayoutAttribute } from '../ast/layout/attribute';
import { AstLayoutAttributeType } from '../ast/layout/attribute_type';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { parserParseLayoutAttributeValue } from './layout_attribute_value';

export function parserParseLayoutAttribute(parser: Parser, element_name: string, tokens: Token[]): AstLayoutAttribute | undefined {
    // key = value
    //      ^
    if (tokens.length === 0) {
        parser.pushError("Unexpected end of tokens in layout attribute");
    }

    const key: string = arrayName2String(tokens);

    const value: AstExpression | undefined = parserParseLayoutAttributeValue(parser, element_name, key);
    if (! value) {
        return undefined;
    }

    return new AstLayoutAttribute(element_name, key, value, AstLayoutAttributeType.Normal);
};
