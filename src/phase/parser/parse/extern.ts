import { Parser } from './parser';
import { AstBlock } from './ast/block';
import { AstExtern } from './ast/extern';
import { parseType } from './expression/type';
import { AstType } from './ast/expression/type';
import { AstFunctionArgument } from './ast/function/function_argument';
import { parserMessageRenderer } from '../../../common/message/message';
import { ParserMessageKeys } from '../../../common/message/parser/parser';
import { parserParseFunctionArguments } from './function/function_attributes';
import { TokenKeywordType, TokenOperatorType } from '../../lexer/tokenizer/type';

export function parserParseExtern(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    parser.expect(TokenKeywordType.TOKEN_EXTERN);

    if (parser.skip(TokenKeywordType.TOKEN_FN)) {

    }
    if (! parser.currentToken.isKeyword) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID_IDENTIFIER));
        return undefined;
    } else if (parser.currentToken.isDefinedIdentifier) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_RESERVED_IN_SALAM));
        return undefined;
    }

    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (name === undefined) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID));
        return undefined;
    }

    // Eating function name
    parser.next();

    const params: AstFunctionArgument[] | undefined = parserParseFunctionArguments(parser);
    if (! params) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID));
        return undefined;
    }

    let return_type: AstType | undefined = undefined;
    if (parser.skip(TokenOperatorType.TOKEN_MEMBER_POINTER)) {
        return_type = parseType(parser);
        if (return_type === undefined) {
            parser.pushError("Invalid data type as return type of function " + name);
            return undefined;
        }
    }
    
    if (return_type === undefined) {
        return_type = AstType.createVoid();
    }

    parser.expect(TokenOperatorType.TOKEN_COLON);
    const generate_name: string | undefined = parser.currentToken.data?.getValueString();
    if (generate_name === undefined) {
        parser.pushError("Invalid data as generate name of an externed function.");
        return undefined;
    }
    parser.expect(TokenKeywordType.TOKEN_IDENTIFIER);

    const ast: AstExtern = new AstExtern(name, params, return_type, generate_name);
    return ast;
};
