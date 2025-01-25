import { Parser } from '../parser';
import { AstExpression } from '../ast/expression';

export function parserParseLayoutAttributeValue(parser: Parser, key: string): AstExpression | undefined {
    if (parser.currentToken.isValue && parser.currentToken.data !== undefined) {
        const ast: AstExpression = new AstExpression(parser.currentToken.data.getValueString());
        parser.next();
        return ast;
    }

    return undefined;
};
