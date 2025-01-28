import { faMessages } from './fa';
import { enMessages } from './en';
import { LexerMessageKeys } from './lexer';
import { MultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const lexerMessages: MultiLanguageMessages<LexerMessageKeys> = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
