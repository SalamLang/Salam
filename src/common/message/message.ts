import { LanguageID } from './../../common/language/language';

// Lexer
export type LexerMessageStruct = {
    [K in LexerMessageKeys]: string;
};

export type LexerMultiLanguageMessages = {
    [L in LanguageID]: LexerMessageStruct;
};

export enum LexerMessageKeys {
    LEXER_INVALID_UNEXPECTED_CHAR,
    LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK,
    LEXER_UNCLOSED_STRING_LITERAL,
};

// Parser
export type ParserMessageStruct = {
    [K in ParserMessageKeys]: string;
};

export type ParserMultiLanguageMessages = {
    [L in LanguageID]: ParserMessageStruct;
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

    PARSER_EXPECTED_TOKEN_TYPE_BUT_GOT,
};

// Validator
export type ValidatorMessageStruct = {
    [K in ValidatorMessageKeys]: string;
};

export type ValidatorMultiLanguageMessages = {
    [L in LanguageID]: ValidatorMessageStruct;
};

export enum ValidatorMessageKeys {
    VALIDATOR_ATTRIBUTE_REQUIRED,
    VALIDATOR_ELEMENT_NOT_VALID,
    VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID,
    VALIDATOR_ATTRIBUTE_NOT_VALID,
    VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID,
    VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY,
};

// Generator
export type GeneratorMessageStruct = {
    [K in GeneratorMessageKeys]: string;
};

export type GeneratorMultiLanguageMessages = {
    [L in LanguageID]: GeneratorMessageStruct;
};

export enum GeneratorMessageKeys {
    GENERATOR_CANNOT_OUTDENT_BELOW_ZERO,
    GENERATOR_UNKNOWN_ELEMENT_TYPE,
};

export function messageRenderer(message: string, ...args: string[]): string {
    return message.replace(/{(\d+)}/g, (match, number) => {
        return typeof args[number] !== "undefined" ? args[number] : match;
    });
};
