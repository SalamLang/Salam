import { Parser } from '../parser';
import { AstExpression } from '../ast/expression/expression';
import { RuntimeElement } from '../../../../runtime/element';
import { RuntimeElementAttribute } from '../../../../runtime/element_attribute';

export function parserParseLayoutAttributeValue(parser: Parser, elementName: string, runtimeElement: RuntimeElement, key: string, runtimeElementAttribute: RuntimeElementAttribute): AstExpression | undefined {
    if (parser.currentToken.isValue && parser.currentToken.data !== undefined) {
        const ast: AstExpression = new AstExpression(parser.currentToken.data.getValueString());
        parser.next();
        return ast;
    }

    return undefined;
};
