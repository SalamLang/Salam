import { faMessages } from './fa';
import { enMessages } from './en';
import { LanguageID } from '../../language/language';
import { GeneratorMultiLanguageMessages } from '../message';

export const generatorMessages: GeneratorMultiLanguageMessages = {
    [LanguageID.LanguageEnglish]: enMessages,
    [LanguageID.LanguagePersian]: faMessages
};
