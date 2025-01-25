import { Parser } from '../parser';
import { AstBlock } from '../ast/block';
import { parserParseBlock } from './block';
import { AstFunctionAttribute } from '../ast/function_attribute';
import { TokenKeywordType } from './../../../lexer/tokenizer/type';
import { AstFunctionDeclaration } from '../ast/function_declaration';
import { parserParseFunctionAttributes } from './function_attributes';

export function parserParseFunction(parser: Parser): AstFunctionDeclaration | undefined {
    parser.expect(TokenKeywordType.TOKEN_FN);

    if (!parser.currentToken.isKeyword) {
        parser.error("Function name is not valid, it should be an identifier string name.");
        return undefined;
    } else if (parser.currentToken.isDefinedIdentifier) {
        parser.error("Function name is not valid, this name already reserved in Salam, you should choose another name.");
        return undefined;
    }

    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (! name) {
        parser.error("Function name is not valid.");
        return undefined;
    }

    const params: AstFunctionAttribute[] | undefined = parserParseFunctionAttributes(parser);
    if (! params) {
        parser.error("Function parameters are not valid.");
        return undefined;
    }

    const body: AstBlock | undefined = parserParseBlock(parser);
    if (! body) {
        parser.error("Function body is not valid.");
        return undefined;
    }

    const ast: AstFunctionDeclaration = new AstFunctionDeclaration(name, params, body);
    return ast;
};
