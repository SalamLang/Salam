import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { Token } from '../../../lexer/tokenizer/token';
import { AstExpressionPostfix } from '../ast/expression/postfix';

export function parseExpresssionPostfix(parser: Parser, result: AstExpression): AstExpression | undefined {
    const op: Token = parser.currentToken;
    if (op.type !== TokenOperatorType.TOKEN_NOT) {
        parser.pushError("Expected postfix operator but got '" + op.enduser_token + "'");
        return undefined;
    }

    const ast: AstExpressionPostfix = new AstExpressionPostfix(op.type, result);
    return ast;
};
