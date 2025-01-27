import { lexerMessages } from './lexer/lexer';
import { parserMessages } from './parser/parser';
import { generatorMessages } from './generator/generator';
import { validatorMessages } from './validator/validator';
import { LanguageID } from './../../common/language/language';

export type MessageStruct<T> = { [K in keyof T]: string };
export type MultiLanguageMessages<T> = { [L in LanguageID]: MessageStruct<T> };

export enum MessageKeys {
    LEXER_PREFIX,
    LEXER_INVALID_UNEXPECTED_CHAR,
    LEXER_UNTERMINATED_MULTI_LINE_COMMENT_BLOCK,
    LEXER_UNCLOSED_STRING_LITERAL,

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

    VALIDATOR_PREFIX,
    VALIDATOR_ATTRIBUTE_REQUIRED,
    VALIDATOR_ELEMENT_NOT_VALID,
    VALIDATOR_ATTRIBUTE_RESERVED_VALUE_NOT_VALID,
    VALIDATOR_ATTRIBUTE_NOT_VALID,
    VALIDATOR_ATTRIBUTE_VALUE_NOT_VALID,
    VALIDATOR_ATTRIBUTE_VALUE_NOT_EMPTY,

    GENERATOR_PREFIX,
    GENERATOR_CANNOT_OUTDENT_BELOW_ZERO,
    GENERATOR_UNKNOWN_ELEMENT_TYPE,
    GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED,
    GENERATOR_INCLUDE_FILE_NOT_FOUND,
    GENERATOR_INCLUDE_FILE_READ_ERROR,
};

export type LexerMessageKeys = typeof lexerMessages;
export type ParserMessageKeys = typeof parserMessages;
export type ValidatorMessageKeys = typeof validatorMessages;
// export type GeneratorMessageKeys = typeof generatorMessages;
export type GeneratorMessageKeys = {
    GENERATOR_PREFIX: string;
    GENERATOR_CANNOT_OUTDENT_BELOW_ZERO: string;
    GENERATOR_UNKNOWN_ELEMENT_TYPE: string;
    GENERATOR_INCLUDE_FILE_PATH_NOT_PROVIDED: string;
    GENERATOR_INCLUDE_FILE_NOT_FOUND: string;
    GENERATOR_INCLUDE_FILE_READ_ERROR: string;
};

export type MessageType = 'LEXER' | 'PARSER' | 'VALIDATOR' | 'GENERATOR';

const prefixMap: Record<MessageType, MessageKeys> = {
    LEXER: MessageKeys.LEXER_PREFIX,
    PARSER: MessageKeys.PARSER_PREFIX,
    VALIDATOR: MessageKeys.VALIDATOR_PREFIX,
    GENERATOR: MessageKeys.GENERATOR_PREFIX,
};

export function getMessagePrefix(
    messages: MultiLanguageMessages<any>,
    language: LanguageID,
    messageKey: MessageKeys
): string {
    const messageType = Object.keys(prefixMap).find((type) =>
        Object.values(messages).some((lang) => lang[messageKey])
    ) as keyof typeof prefixMap;

    if (!messageType) {
        return '';
    }

    console.log(prefixMap);
    const keyword_id = prefixMap[messageType];

    const keyword_value = keyword_id;// + "_PREFIX";

    console.log(MessageKeys);
    console.log(messageType);
    console.log(keyword_id);
    console.log(keyword_value);
    console.log(messages[language]);

    return messages[language][keyword_value];
};


export function messageRenderer(
    messages: MultiLanguageMessages<any>,
    language: LanguageID,
    messageKey: MessageKeys,
    ...args: string[]
): string {
    const messagePrefix = getMessagePrefix(messages, language, messageKey);

    const messageTemplate = messages[language][messageKey];
    if (!messageTemplate) throw new Error('Invalid messageKey provided');

    return messagePrefix + messageTemplate.replace(/{(\d+)}/g, (_, number) => args[number] || _);
};
