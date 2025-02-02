import { Parser } from '../parser';
import { parseExpression } from '../expression/expression';
import { AstExpression } from '../ast/expression/expression';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { AstFunctionParameter } from '../ast/function/function_parameter';
import { AstExpressionFunctionCall } from '../ast/function/function_call';

export function parseExpressionFunctionCall(parser: Parser, functionName: AstExpression): AstExpressionFunctionCall | undefined {
    const parameters: AstFunctionParameter[] = [];
   
    // Consume `(` token
    parser.next();

    // If not an empty argument list, parse arguments
    if (!parser.has(TokenOperatorType.TOKEN_RIGHT_PAREN)) {
        do {
            const parameter_value: AstExpression | undefined = parseExpression(parser);
            if (parameter_value === undefined) {
                parser.pushError("Expected an expression as function argument.");
                break;
            }
            const parameter: AstFunctionParameter = new AstFunctionParameter(parameter_value);
            parameters.push(parameter);
        } while (parser.skip(TokenOperatorType.TOKEN_COMMA));
        
        // Ensure closing `)`
        if (!parser.skip(TokenOperatorType.TOKEN_RIGHT_PAREN)) {
            parser.pushError("Expected closing parenthesis `)` after function arguments.");
        }
    } else {
        // Consume `)`
        parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN);
    }

    return new AstExpressionFunctionCall(functionName, parameters);
}
