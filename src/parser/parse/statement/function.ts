import { Parser } from './../parser';
import { AstBlock } from './../ast/block';
import { parserParseBlock } from './block';
import { AstFunctionParameter } from './../ast/function_parameter';
import { TokenKeywordType } from './../../../lexer/tokenizer/type';
import { AstFunctionDeclaration } from './../ast/function_declaration';

export function parserParseFunction(parser: Parser): AstFunctionDeclaration | undefined {
    parser.expect(TokenKeywordType.TOKEN_FN);
    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (!name) {
        parser.error("Function name is not valid.");
        return undefined;
    }
    const params: AstFunctionParameter[] = [];

    const body: AstBlock | undefined = parserParseBlock(parser);
    if (! body) {
        parser.error("Function body is not valid.");
        return undefined;
    }

    const ast: AstFunctionDeclaration = new AstFunctionDeclaration(name, params, body);
    return ast;
};
