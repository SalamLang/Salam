import { Parser } from '../parser';
import { AstExpression } from '../ast/expression/expression';
import { parseExpression } from '../expression/expression';

export function parserParseLayoutAttributeValue(parser: Parser, element_enduser_name: string, key: string): AstExpression | undefined {
    return parseExpression(parser);
};
