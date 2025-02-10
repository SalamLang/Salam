import { Parser } from './../parser';
import { parseType } from '../expression/type';
import { AstType } from '../ast/expression/type';
import { Token } from '../../../lexer/tokenizer/token';
import { parseExpression } from '../expression/expression';
import { AstExpression } from '../ast/expression/expression';
import { TokenKeywordType, TokenOperatorType } from '../../../lexer/tokenizer/type';
import { AstStatementExpressionDeclaration } from '../ast/statement/expression_declaration';

export function parseStatementExpressionDeclaration(parser: Parser): AstStatementExpressionDeclaration | undefined {
    const type: AstType | undefined = parseType(parser);
    if (type === undefined) {
        return undefined;
    }

    const name: Token | undefined = parser.expectGet(TokenKeywordType.TOKEN_IDENTIFIER);
    if (name === undefined || name.data === undefined) {
        return undefined;
    }
    parser.expect(TokenOperatorType.TOKEN_ASSIGN);
    const value: AstExpression | undefined = parseExpression(parser);
    if (value === undefined) {
        return undefined;
    }

    const ast: AstStatementExpressionDeclaration = new AstStatementExpressionDeclaration(type, name.data.getValueString(), value);
    return ast;
};
