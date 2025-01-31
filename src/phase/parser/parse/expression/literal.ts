import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';
import { AstExpressionLiteral } from '../ast/expression/literal';
import { AstExpressionVariable } from '../ast/expression/variable';
import { TokenKeywordType, TokenValueType } from '../../../lexer/tokenizer/type';

export function parseExpressionLiteral(parser: Parser): AstExpression | undefined {
    const currentToken = parser.currentToken;
    if (currentToken?.data !== undefined) {
        parser.next();
        switch (currentToken.type) {
            case TokenKeywordType.TOKEN_IDENTIFIER:
                const varable_identifier: AstExpression = new AstExpression(currentToken.data.getValueString());
                return new AstExpressionVariable(varable_identifier);

            case TokenValueType.TOKEN_STRING:
            case TokenValueType.TOKEN_BOOL_FALSE:
            case TokenValueType.TOKEN_BOOL_TRUE:
            case TokenValueType.TOKEN_NUMBER_FLOAT:
            case TokenValueType.TOKEN_NUMBER_INT:
                return new AstExpressionLiteral(currentToken.data.getValueString(), currentToken.type);
        }
    }
    return undefined;
};
