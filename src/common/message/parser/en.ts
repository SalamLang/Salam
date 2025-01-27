import { ParserMessageKeys, ParserMessageStruct } from './../message';

export const enMessages: ParserMessageStruct = {
    [ParserMessageKeys.PARSER_PREFIX]: "Parser error: ",
    [ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION_STATEMENT]: "Failed to parse function staement.",
    [ParserMessageKeys.PARSER_FAILED_TO_PARSE_FUNCTION]: "Failed to parse function.",
    [ParserMessageKeys.PARSER_FAILED_TO_PARSE_LAYOUT_ELEMENT]: "Failed to parse layout element.",
    [ParserMessageKeys.PARSER_DUPLICATE_LAYOUT_DEFINITION]: "Duplicate layout definition, cannot have more than one layout definition in a program.",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_PROGRAM]: "Unexpected token in program, current token is '{0}'",

    [ParserMessageKeys.PARSER_UNEXPECTED_END_OF_TOKENS_IN_LAYOUT_ATTRIBUTE]: "Unexpected end of tokens in layout attribute",

    [ParserMessageKeys.PARSER_DUPLICATE_ATTRIBUTE_IN_LAYOUT]: "Duplicate attribute '{0}' in layout",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_AS_ELEMENT_NAME]: "Unexpected token as element name, current token is '{0}'",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT]: "Unexpected token in layout, current token is '{0}'",
    [ParserMessageKeys.PARSER_UNEXPECTED_TOKEN_IN_LAYOUT_AS_ATTRIBUTE]: "Unexpected token in layout as attribute, current token is '{0}'",

    [ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID_IDENTIFIER]: "Function name is not valid, it should be an identifier string name.",
    [ParserMessageKeys.PARSER_FUNCTION_NAME_IS_RESERVED_IN_SALAM]: "Function name is not valid, this name already reserved in Salam, you should choose another name.",
    [ParserMessageKeys.PARSER_FUNCTION_NAME_IS_NOT_VALID]: "Function name is not valid.",
    [ParserMessageKeys.PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID]: "Function parameters are not valid.",
    [ParserMessageKeys.PARSER_FUNCTION_BODY_IS_NOT_VALID]: "Function body is not valid.",

    [ParserMessageKeys.PARSER_EXPECTED_TOKEN_TYPE_BUT_GOT]: "Expected token type {0}, but got {1}.",
};
