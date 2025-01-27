import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';

export function parserParseLayoutAttributeValue(parser: Parser, element_enduser_name: string, key: string): AstExpression | undefined {
    if (parser.currentToken.isValue && parser.currentToken.data !== undefined) {
        const ast: AstExpression = new AstExpression(parser.currentToken.data.getValueString());
        parser.next();
        return ast;
    }

    return undefined;
};
