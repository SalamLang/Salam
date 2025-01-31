import { Parser } from './../parser';
import { AstExpression } from './../ast/expression/expression';
import { TokenKeywordType, TokenValueType } from '../../../lexer/tokenizer/type';
import { AstExpressionVariable } from '../ast/expression/variable';

export function parseExpressionLiteral(parser: Parser): AstExpression | undefined {
    const currentToken = parser.currentToken;
    if (currentToken?.data !== undefined) {
        parser.next();
        switch (currentToken.type) {
            case TokenKeywordType.TOKEN_IDENTIFIER:
                const varable_identifier: AstExpression = new AstExpression(currentToken.data.getValueString());
                return new AstExpressionVariable(varable_identifier);

            case TokenValueType.TOKEN_STRING:
                return new AstExpression(currentToken.data.getValueString());

            case TokenValueType.TOKEN_BOOL_FALSE:
                return new AstExpression(currentToken.data.getValueString());
                
            case TokenValueType.TOKEN_BOOL_TRUE:
                return new AstExpression(currentToken.data.getValueString());
                
            case TokenValueType.TOKEN_NUMBER_FLOAT:
                return new AstExpression(currentToken.data.getValueString());
                
            case TokenValueType.TOKEN_NUMBER_INT:
                return new AstExpression(currentToken.data.getValueString());
        }
    }
    return undefined;
};
