import { Parser } from './../parser';
import { Token } from '../../../lexer/tokenizer/token';
import { AstExpression } from './../ast/expression/expression';
import { TokenKeywordType, TokenValueType } from '../../../lexer/tokenizer/type';
import { AstExpressionVariable } from '../ast/expression/variable';
import { AstExpressionLiteral } from '../ast/expression/literal';
import { AstType } from '../ast/expression/type';

export function parseExpressionLiteral(parser: Parser): AstExpression | undefined {
    const currentToken: Token = parser.currentToken;
    if (currentToken?.data !== undefined) {
        parser.next();
        switch (currentToken.type) {
            case TokenKeywordType.TOKEN_IDENTIFIER:
                const varable_identifier: AstExpression = new AstExpressionLiteral(currentToken.data.getValueString(), AstType.createIdentifier());
                return new AstExpressionVariable(varable_identifier);

            case TokenValueType.TOKEN_STRING:
                return new AstExpressionLiteral(currentToken.data.getValueString(), AstType.createString());

            case TokenValueType.TOKEN_BOOL_FALSE:
                return new AstExpressionLiteral("TRUE", AstType.createBool());

            case TokenValueType.TOKEN_BOOL_TRUE:
                return new AstExpressionLiteral("TRUE", AstType.createBool());

            case TokenValueType.TOKEN_NUMBER_FLOAT:
                return new AstExpressionLiteral(currentToken.data.getValueString(), AstType.createFloat());

            case TokenValueType.TOKEN_NUMBER_INT:
                return new AstExpressionLiteral(currentToken.data.getValueString(), AstType.createInt());
        }
    }
    return undefined;
}

