import { faMessages } from './fa';
import { enMessages } from './en';
import { LexerMultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const lexerMessages: LexerMultiLanguageMessages = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
