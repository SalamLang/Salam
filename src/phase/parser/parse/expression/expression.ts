import { Parser } from './../parser';
import { Token } from './../../../lexer/tokenizer/token';
import { AstExpression } from './../ast/expression/expression';
import { TokenValueType } from '../../../lexer/tokenizer/type';

export function parseExpression(parser: Parser): AstExpression | undefined {
    const current_token: Token | undefined = parser.currentToken;
    const isValuePrimitive: boolean = Object.values(TokenValueType).includes(current_token.type as TokenValueType);

    if (current_token.data !== undefined && isValuePrimitive) {
        parser.next();
        const ast: AstExpression = new AstExpression(current_token.data.getValueString());
        return ast;
    }
    // else if (current_token.type === TokenValueType.TOKEN_NUMBER_FLOAT) {
    //     parser.next();
    //     const ast: AstExpression = new AstExpression(current_token.value);
    //     return ast;
    // } else if (current_token.type === TokenValueType.TOKEN_NUMBER_INT) {
    //     parser.next();
    //     const ast: AstExpression = new AstExpression(current_token.value);
    //     return ast;
    // } else if (current_token.type === TokenValueType.TOKEN_BOOL_TRUE) {
    //     parser.next();
    //     const ast: AstExpression = new AstExpression(current_token.value);
    //     return ast;
    // } else if (current_token.type === TokenValueType.TOKEN_BOOL_FALSE) {
    //     parser.next();
    //     const ast: AstExpression = new AstExpression(current_token.value);
    //     return ast;
    // }
    return undefined;
};
