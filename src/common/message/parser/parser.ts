import { faMessages } from './fa';
import { enMessages } from './en';
import { MultiLanguageMessages } from '../message';
import { LanguageID } from '../../language/language';

export const parserMessages: MultiLanguageMessages = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
