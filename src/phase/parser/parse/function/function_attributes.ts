import { Parser } from './../parser';
import { AstType } from '../ast/expression/type';
import { parseType } from '../expression/type';
import { Token } from '../../../lexer/tokenizer/token';
import { TokenOperatorType } from '../../../lexer/tokenizer/type';
import { AstFunctionArgument } from '../ast/function/function_argument';

export function parserParseFunctionArguments(parser: Parser): AstFunctionArgument[] | undefined {
    const args: AstFunctionArgument[] = [];

    if (!parser.skip(TokenOperatorType.TOKEN_LEFT_PAREN)) {
        return args;
    }
    
    while (!parser.isEnd) {
        if (parser.currentToken.type === TokenOperatorType.TOKEN_RIGHT_PAREN) {
            break;
        }
        const type: AstType | undefined = parseType(parser);
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

        const argument: AstFunctionArgument = new AstFunctionArgument(name, type);
        args.push(argument);
    }

    parser.expect(TokenOperatorType.TOKEN_RIGHT_PAREN);
    
    return args;
};
