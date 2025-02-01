import { Parser } from './../parser';
import { AstType } from '../ast/expression/type';
import { parserParseType } from '../expression/type';
import { Token } from '../../../lexer/tokenizer/token';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { AstFunctionParameter } from '../ast/function_parameter';

export function parserParseFunctionAttributes(parser: Parser): AstFunctionParameter[] | undefined {
    const params: AstFunctionParameter[] = [];

    if (!parser.skip(TokenOperatorType.TOKEN_LEFT_PAREN)) {
        return params;
    }
    
    while (!parser.isEnd) {
        const type: AstType | undefined = parserParseType(parser);
        const token: Token = parser.currentToken;
        if (type === undefined) {
            parser.pushError("function attribute type is not valid!");
            return undefined;
        }
        if (! token.isKeyword || token.data === undefined) {
            parser.pushError("we expect attribute name for the function, but is invalid.");
            return undefined;
        }
        const name: string = token.data.getValueString();
        parser.next(); // Eating attribute name

        const attribute: AstFunctionParameter = new AstFunctionParameter(name, type);
        params.push(attribute);
        if (parser.currentToken.type === TokenOperatorType.TOKEN_RIGHT_PAREN) {
            break;
        }
    }

    parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN);
    
    return params;
};
