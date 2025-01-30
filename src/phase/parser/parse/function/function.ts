import { Parser } from './../parser';
import { AstBlock } from './../ast/block';
import { parserParseBlock } from './../block';
import { AstFunctionAttribute } from './../ast/function_attribute';
import { TokenKeywordType } from './../../../lexer/tokenizer/type';
import { parserParseFunctionAttributes } from './function_attributes';
import { parserMessageRenderer } from './../../../../common/message/message';
import { ParserMessageKeys } from './../../../../common/message/parser/parser';
import { AstFunctionDeclaration } from './../ast/statement/function_declaration';

export function parserParseFunction(parser: Parser): AstFunctionDeclaration | undefined {
    console.log("parserParseFunction");
    parser.expect(TokenKeywordType.TOKEN_FN);

    if (! parser.currentToken.isKeyword) {
        parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID_IDENTIFIER));
        return undefined;
    } else if (parser.currentToken.isDefinedIdentifier) {
        parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_FUNCTION_NAME_IS_RESERVED_IN_SALAM));
        return undefined;
    }

    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (! name) {
        parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID));
        return undefined;
    }

    // Eating function name
    parser.next();

    const params: AstFunctionAttribute[] | undefined = parserParseFunctionAttributes(parser);
    if (! params) {
        parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID));
        return undefined;
    }

    const body: AstBlock | undefined = parserParseBlock(parser);
    if (! body) {
        parser.pushError(parserMessageRenderer(parser.lexer.language.id, ParserMessageKeys.PARSER_FUNCTION_BODY_IS_NOT_VALID));
        return undefined;
    }

    const ast: AstFunctionDeclaration = new AstFunctionDeclaration(name, params, body);
    return ast;
};
