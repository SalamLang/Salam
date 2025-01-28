import { lexerMessages } from './lexer/message';
import { LexerMessageKeys } from './lexer/lexer';
import { parserMessages } from './parser/message';
import { ParserMessageKeys } from './parser/parser';
import { validatorMessages } from './validator/message';
import { generatorMessages } from './generator/message';
import { GeneratorMessageKeys } from './generator/generator';
import { ValidatorMessageKeys } from './validator/validator';
import { LanguageID } from './../../common/language/language';

export type MessageStruct<T> = { [K in keyof T]: string };
export type MultiLanguageMessages<T extends string | number | symbol> = {
  [L in LanguageID]: Record<T, string>;
};

export type MessageType = 'LEXER' | 'PARSER' | 'VALIDATOR' | 'GENERATOR';

function messageRenderer<T extends string | number | symbol>(
  type: MessageType,
  messages: MultiLanguageMessages<T>,
  language: LanguageID,
  messageKey: T,
  ...args: string[]
): string {
  const prefix = getMessagePrefix(type, language, messages);
  const messageTemplate = messages[language]?.[messageKey];

  if (!messageTemplate) {
    throw new Error(
      `Message key "${String(messageKey)}" not found for type "${type}" in language "${language}".`
    );
  }

  return prefix + messageTemplate.replace(/{(\d+)}/g, (_, index) => args[index] || `{${index}}`);
}

function getMessagePrefix<T extends string | number | symbol>(
  type: MessageType,
  language: LanguageID,
  messages: MultiLanguageMessages<T>
): string {
  const prefixKey = `${type}_PREFIX` as T;
  return messages[language]?.[prefixKey] || '';
}

export const lexerMessageRenderer = (
  language: LanguageID,
  messageKey: LexerMessageKeys,
  ...args: string[]
): string => messageRenderer('LEXER', lexerMessages, language, messageKey, ...args);

export const parserMessageRenderer = (
  language: LanguageID,
  messageKey: ParserMessageKeys,
  ...args: string[]
): string => messageRenderer('PARSER', parserMessages, language, messageKey, ...args);

export const validatorMessageRenderer = (
  language: LanguageID,
  messageKey: ValidatorMessageKeys,
  ...args: string[]
): string => messageRenderer('VALIDATOR', validatorMessages, language, messageKey, ...args);

export const generatorMessageRenderer = (
  language: LanguageID,
  messageKey: GeneratorMessageKeys,
  ...args: string[]
): string => messageRenderer('GENERATOR', generatorMessages, language, messageKey, ...args);
