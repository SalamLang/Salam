import { lexerMessages } from './lexer/lexer';
import { parserMessages } from './parser/parser';
import { generatorMessages } from './generator/generator';
import { validatorMessages } from './validator/validator';
import { LanguageID } from './../../common/language/language';

// Lexer
export type LexerMessageStruct = {
    [K in LexerMessageKeys]: string;
};

export type LexerMultiLanguageMessages = {
    [L in LanguageID]: LexerMessageStruct;
};

export enum LexerMessageKeys {
    LEXER_PREFIX,
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
    PARSER_PREFIX,
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
    VALIDATOR_PREFIX,
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
    GENERATOR_PREFIX,
    GENERATOR_CANNOT_OUTDENT_BELOW_ZERO,
    GENERATOR_UNKNOWN_ELEMENT_TYPE,
    GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED,
    GENERATOR_INCLUDE_FILE_NOT_FOUND,
    GENERATOR_INCLUDE_FILE_READ_ERROR,
};

export type MessageListType = LexerMessageKeys | ParserMessageKeys | ValidatorMessageKeys | GeneratorMessageKeys;

export enum MessageType {
    MESSAGE_TYPE_LEXER,
    MESSAGE_TYPE_PARSER,
    MESSAGE_TYPE_VALIDATOR,
    MESSAGE_TYPE_GENERATOR,
};

type PrefixKeyMap = {
    [MessageType.MESSAGE_TYPE_LEXER]: keyof LexerMessageStruct;
    [MessageType.MESSAGE_TYPE_PARSER]: keyof ParserMessageStruct;
    [MessageType.MESSAGE_TYPE_VALIDATOR]: keyof ValidatorMessageStruct;
    [MessageType.MESSAGE_TYPE_GENERATOR]: keyof GeneratorMessageStruct;
};

function getMessagePrefix(
    messages: GeneratorMultiLanguageMessages | LexerMultiLanguageMessages | ParserMultiLanguageMessages | ValidatorMultiLanguageMessages,
    language: LanguageID,
    messageKey: MessageListType
): string {
    const prefixKeyMap: PrefixKeyMap = {
        [MessageType.MESSAGE_TYPE_LEXER]: LexerMessageKeys.LEXER_PREFIX,
        [MessageType.MESSAGE_TYPE_PARSER]: ParserMessageKeys.PARSER_PREFIX,
        [MessageType.MESSAGE_TYPE_VALIDATOR]: ValidatorMessageKeys.VALIDATOR_PREFIX,
        [MessageType.MESSAGE_TYPE_GENERATOR]: GeneratorMessageKeys.GENERATOR_PREFIX,
    };

    let messageType: MessageType;
    if (Object.values(LexerMessageKeys).includes(messageKey as LexerMessageKeys)) {
        messageType = MessageType.MESSAGE_TYPE_LEXER;
    } else if (Object.values(ParserMessageKeys).includes(messageKey as ParserMessageKeys)) {
        messageType = MessageType.MESSAGE_TYPE_PARSER;
    } else if (Object.values(ValidatorMessageKeys).includes(messageKey as ValidatorMessageKeys)) {
        messageType = MessageType.MESSAGE_TYPE_VALIDATOR;
    } else if (Object.values(GeneratorMessageKeys).includes(messageKey as GeneratorMessageKeys)) {
        messageType = MessageType.MESSAGE_TYPE_GENERATOR;
    } else {
        return '';
    }

    return (messages as any)[language][prefixKeyMap[messageType]] || '';
};

export function messageRenderer(
    messages: GeneratorMultiLanguageMessages | LexerMultiLanguageMessages | ParserMultiLanguageMessages | ValidatorMultiLanguageMessages,
    language: LanguageID,
    messageKey: MessageListType,
    ...args: string[]
): string {
    const messagePrefix = getMessagePrefix(messages, language, messageKey);

    let messageTemplate: string;
    
    if (Object.values(LexerMessageKeys).includes(messageKey as LexerMessageKeys)) {
        messageTemplate = (messages as LexerMultiLanguageMessages)[language][messageKey as LexerMessageKeys];
    } else if (Object.values(ParserMessageKeys).includes(messageKey as ParserMessageKeys)) {
        messageTemplate = (messages as ParserMultiLanguageMessages)[language][messageKey as ParserMessageKeys];
    } else if (Object.values(ValidatorMessageKeys).includes(messageKey as ValidatorMessageKeys)) {
        messageTemplate = (messages as ValidatorMultiLanguageMessages)[language][messageKey as ValidatorMessageKeys];
    } else if (Object.values(GeneratorMessageKeys).includes(messageKey as GeneratorMessageKeys)) {
        messageTemplate = (messages as GeneratorMultiLanguageMessages)[language][messageKey as GeneratorMessageKeys];
    } else {
        throw new Error('Invalid messageKey provided');
    }

    return messagePrefix + messageTemplate.replace(/{(\d+)}/g, (match, number) => {
        return typeof args[number] !== "undefined" ? args[number] : match;
    });
};
