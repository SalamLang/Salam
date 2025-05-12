import { faMessages } from './fa';
import { enMessages } from './en';
import { ParserMessageKeys } from './parser';
import { MultiLanguageMessages } from './../message';
import { LanguageID } from './../../language/language';

export const parserMessages: MultiLanguageMessages<ParserMessageKeys> = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
