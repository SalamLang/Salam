import { Parser } from './../parser';
import { AstExpression } from './../ast/expression';
import { AstLayoutAttribute } from './../ast/layout/attribute';
import { AstLayoutAttributeType } from './../ast/layout/attribute_type';
import { Token, arrayName2String } from './../../../lexer/tokenizer/token';
import { parserParseLayoutAttributeValue } from './layout_attribute_value';

export function parserParseLayoutAttribute(parser: Parser, tokens: Token[]): AstLayoutAttribute | undefined {
    // key = value
    //      ^
    if (tokens.length === 0) {
        parser.error("Unexpected end of tokens in layout attribute");
    }

    const key: string = arrayName2String(tokens);

    const value: AstExpression | undefined = parserParseLayoutAttributeValue(parser, key);
    if (! value) {
        return undefined;
    }

    return new AstLayoutAttribute(key, value, AstLayoutAttributeType.Normal);
};
