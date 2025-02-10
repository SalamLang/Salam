import { Parser } from './../parser';
import { parseExpression } from './expression';
import { Token } from '../../../lexer/tokenizer/token';
import { AstExpression } from './../ast/expression/expression';
import { AstExpressionPrefix } from '../ast/expression/prefix';
import { AstExpressionPostfix } from '../ast/expression/postfix';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';

export function parseExpressionPrefix(parser: Parser, min_bp: number): AstExpression | undefined {
    const op: Token = parser.currentToken;
    if (op.type !== TokenOperatorType.TOKEN_PLUS && op.type !== TokenOperatorType.TOKEN_MINUS) {
        parser.pushError("Expected prefix operator but got '" + op.enduser_token + "'");
        return undefined;
    }
    
    parser.next();

    const right: AstExpression | undefined = parseExpression(parser, min_bp);
    if (right === undefined) {
        parser.pushError("Expected expression after prefix operator but is not valid");
        return undefined;
    }

    const ast: AstExpressionPostfix = new AstExpressionPrefix(op.type, right);
    return ast;
};
