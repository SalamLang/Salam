import { LanguageID } from './../../common/language/language';

export enum LexerMessageKeys {
    LEXER_INVALID_UNEXPECTED_CHAR,
    LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK,
};

export enum ParserMessageKeys {
    PARSER_FAILED_TO_PARSE_FUNCTION_STATEMENT,
    PARSER_FAILED_TO_PARSE_FUNCTION,
    PARSER_FAILED_TO_PARSE_LAYOUT_ELEMENT,
    PARSER_DUPLICATE_LAYOUT_DEFINITION,
    PARSER_UNEXPECTED_TOKEN_IN_PROGRAM,

    PARSER_UNEXPECTED_END_OF_TOKENS_IN_LAYOUT_ATTRIBUTE,

    PARSER_DUPLICATE_ATTRIBUTE_IN_LAYOUT,
    PARSER_UNEXPECTED_TOKEN_AS_ELEMENT_NAME,
    PARSER_UNEXPECTED_TOKEN_IN_LAYOUT,
    PARSER_UNEXPECTED_TOKEN_IN_LAYOUT_AS_ATTRIBUTE,

    PARSER_FUNCTION_NAME_IS_NOT_VALID_IDENTIFIER,
    PARSER_FUNCTION_NAME_IS_RESERVED_IN_SALAM,
    PARSER_FUNCTION_NAME_IS_NOT_VALID,
    PARSER_FUNCTION_PARAMETERS_ARE_NOT_VALID,
    PARSER_FUNCTION_BODY_IS_NOT_VALID,
};

export type LexerMessageStruct = {
    [K in LexerMessageKeys]: string;
};

export type LexerMultiLanguageMessages = {
    [L in LanguageID]: LexerMessageStruct;
};

export type ParserMessageStruct = {
    [K in ParserMessageKeys]: string;
};

export type ParserMultiLanguageMessages = {
    [L in LanguageID]: ParserMessageStruct;
};

export function messageRenderer(message: string, ...args: string[]): string {
    return message.replace(/{(\d+)}/g, (match, number) => {
        return typeof args[number] !== "undefined" ? args[number] : match;
    });
}
