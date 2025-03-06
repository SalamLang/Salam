import { Parser } from './parser';
import { AstBlock } from './ast/block';
import { AstExtern } from './ast/extern';
import { parseType } from './expression/type';
import { AstType } from './ast/expression/type';
import { AstExternType } from './ast/extern_type';
import { Token } from '../../lexer/tokenizer/token';
import { LanguageID } from '../../../common/language/language';
import { AstFunctionArgument } from './ast/function/function_argument';
import { parserMessageRenderer } from '../../../common/message/message';
import { ParserMessageKeys } from '../../../common/message/parser/parser';
import { parserParseFunctionArguments } from './function/function_attributes';
import { TokenKeywordType, TokenOperatorType, TokenValueType } from '../../lexer/tokenizer/type';

export function parserParseExternHeader(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    const name: Token | undefined = parser.expectGet(TokenValueType.TOKEN_STRING);
    if (name === undefined) {
        parser.pushError("Expected a string value for the extern header but got something else.");
        return undefined;
    }
    const generate_name: string = name?.data?.getValueString() || "";
    const ast: AstExtern = new AstExtern(AstExternType.EXTERN_HEADER, generate_name, undefined, undefined, generate_name);
    return ast;
};

export function parserParseExternLib(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    const name: Token | undefined = parser.expectGet(TokenValueType.TOKEN_STRING);
    if (name === undefined) {
        parser.pushError("Expected a string value for the extern library but got something else.");
        return undefined;
    }
    const generate_name: string = name?.data?.getValueString() || "";
    const ast: AstExtern = new AstExtern(AstExternType.EXTERN_LIB, generate_name, undefined, undefined, generate_name);
    return ast;
};

export function parserParseExternInc(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    const name: Token | undefined = parser.expectGet(TokenValueType.TOKEN_STRING);
    if (name === undefined) {
        parser.pushError("Expected a string value for the extern include but got something else.");
        return undefined;
    }
    const generate_name: string = name?.data?.getValueString() || "";
    const ast: AstExtern = new AstExtern(AstExternType.EXTERN_INC, generate_name, undefined, undefined, generate_name);
    return ast;
};

export function parserParseExternObj(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    const name: Token | undefined = parser.expectGet(TokenValueType.TOKEN_STRING);
    if (name === undefined) {
        parser.pushError("Expected a string value for the extern object but got something else.");
        return undefined;
    }
    const generate_name: string = name?.data?.getValueString() || "";
    const ast: AstExtern = new AstExtern(AstExternType.EXTERN_OBJ, generate_name, undefined, undefined, generate_name);
    return ast;
};

export function parserParseExternFn(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (name === undefined) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID));
        return undefined;
    }
    parser.next();

    let params: AstFunctionArgument[] | undefined = undefined;
    params = parserParseFunctionArguments(parser);
    if (params === undefined) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID));
        return undefined;
    }
    
    let return_type: AstType | undefined = undefined;
    if (parser.skip(TokenOperatorType.TOKEN_MEMBER_POINTER)) {
        return_type = parseType(parser);
        if (return_type === undefined) {
            parser.pushError("Invalid data type as return type of extern function " + name);
            return undefined;
        }
    } else {
        return_type = AstType.createVoid();
    }

    parser.expect(TokenOperatorType.TOKEN_COLON);

    const generate_name: string | undefined = parser.currentToken.data?.getValueString();
    if (generate_name === undefined) {
        parser.pushError("Invalid data as generate name of an externed function");
        return undefined;
    }
    parser.expect(TokenKeywordType.TOKEN_IDENTIFIER);

    const ast: AstExtern = new AstExtern(AstExternType.EXTERN_FN, name, params, return_type, generate_name);
    return ast;
};

export function parserParseExternVar(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    let return_type: AstType | undefined = parseType(parser);
    if (return_type === undefined) {
        parser.pushError("Invalid data type as return type of extern variable");
        return undefined;
    }

    const name: string | undefined = parser.currentToken.data?.getValueString();
    if (name === undefined) {
        parser.pushError(parserMessageRenderer(parser.getLanguageId(), ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID));
        return undefined;
    }
    parser.next();

    parser.expect(TokenOperatorType.TOKEN_COLON);

    const generate_name: string | undefined = parser.currentToken.data?.getValueString();
    if (generate_name === undefined) {
        parser.pushError("Invalid data as generate name of an externed variable");
        return undefined;
    }
    parser.expect(TokenKeywordType.TOKEN_IDENTIFIER);

    const ast: AstExtern = new AstExtern(AstExternType.EXTERN_VAR, name, undefined, return_type, generate_name);
    return ast;
};

export function parserParseExtern(parser: Parser, parent_block: AstBlock): AstExtern | undefined {
    parser.expect(TokenKeywordType.TOKEN_EXTERN);

    if (parser.skip(TokenKeywordType.TOKEN_FN)) {
        return parserParseExternFn(parser, parent_block);
    } else if (parser.skipIdentifiers({[LanguageID.LanguageEnglish]: "library", [LanguageID.LanguagePersian]: "کتابخانه"})) {
        return parserParseExternLib(parser, parent_block);
    } else if (parser.skipIdentifiers({[LanguageID.LanguageEnglish]: "include", [LanguageID.LanguagePersian]: "فراخوانی"})) {
        return parserParseExternInc(parser, parent_block);
    } else if (parser.skipIdentifiers({[LanguageID.LanguageEnglish]: "header", [LanguageID.LanguagePersian]: "هدر"})) {
        return parserParseExternHeader(parser, parent_block);
    } else if (parser.skipIdentifiers({[LanguageID.LanguageEnglish]: "object", [LanguageID.LanguagePersian]: "شی"})) {
        return parserParseExternObj(parser, parent_block);
    } else {
        return parserParseExternVar(parser, parent_block);
    }
};
