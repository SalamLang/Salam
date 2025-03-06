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
    let is_function: boolean = false;
    if (parser.skip(TokenKeywordType.TOKEN_FN)) {
        is_function = true;
    }

    // if (parser.currentToken.isKeyword) {
    //     parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID_IDENTIFIER));
    //     return undefined;
    // } else if (
    //     (is_function === true && parser.currentToken.isDefinedIdentifier === true) ||
    //     (is_function === false && parser.currentToken.isDefinedIdentifier === true)
    // ) {
    //     parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_RESERVED_IN_SALAM));
    //     return undefined;
    // }

    let return_type: AstType | undefined = undefined;
    if (is_function === false) {
        return_type = parseType(parser);
        if (return_type === undefined) {
            parser.pushError("Invalid data type as return type of extern variable");
            return undefined;
        }
    }

    // Eating function name
    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (name === undefined) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID));
        return undefined;
    }
    parser.next();

    // Parameters for function externs
    let params: AstFunctionArgument[] | undefined = undefined;
    if (is_function === true) {
        params = parserParseFunctionArguments(parser);
        if (params === undefined) {
            parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID));
            return undefined;
        }
        
        if (parser.skip(TokenOperatorType.TOKEN_MEMBER_POINTER)) {
            return_type = parseType(parser);
            if (return_type === undefined) {
                parser.pushError("Invalid data type as return type of extern function " + name);
                return undefined;
            }
        } else {
            return_type = AstType.createVoid();
        }
    }

    parser.expect(TokenOperatorType.TOKEN_COLON);
    const generate_name: string | undefined = parser.currentToken.data?.getValueString();
    if (generate_name === undefined) {
        parser.pushError("Invalid data as generate name of an externed " + (is_function ? "function": "variable"));
        return undefined;
    }
    parser.expect(TokenKeywordType.TOKEN_IDENTIFIER);

    if (return_type === undefined) {
        parser.pushError("Invalid data type as return type of extern");
        return undefined;
    }

    const ast: AstExtern = new AstExtern(name, params, return_type, generate_name);
    return ast;
};
