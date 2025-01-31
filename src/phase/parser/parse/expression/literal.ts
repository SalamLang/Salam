import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';

export function parseExpressionLiteral(parser: Parser): AstExpression | undefined {
    console.log("parseExpressionLiteral");
    const currentToken = parser.currentToken;
    if (currentToken?.data !== undefined) {
        parser.next();
        return new AstExpression(currentToken.data.getValueString());
    }
    return undefined;
};
